/*
 * NeovimApi.re
 *
 * Wrapper on-top of MsgPack primitives to handle Neovim's primitives:
 * - notifications
 * - requests
 */

open Oni_Core;
open Rench;

module M = Msgpck;

type requestFunction = (string, M.t) => unit;
type requestSyncFunction = (string, M.t) => M.t;

type response = {
  responseId: int,
  payload: M.t,
};

type notification = {
  notificationType: string,
  payload: M.t,
};

type t = {
  requestSync: requestSyncFunction,
  /*
   * request is a 'fire-and-forget' method -
   * does not block on response.
   */
  request: requestFunction,
  /*
   * Pump should be called periodically to dispatch any queued notifications.
   */
  pump: unit => unit,
  onNotification: Event.t(notification),
  dispose: unit => unit,
};

exception RequestFailed(string);

let currentId = ref(0);

let getNextId = () => {
  let ret = currentId^;
  currentId := currentId^ + 1;
  ret;
};

let withMutex = (m: Mutex.t, f) => {
  Mutex.lock(m);
  let r = f();
  Mutex.unlock(m);
  r;
};

let make = (msgpack: MsgpackTransport.t) => {
  let queuedResponseMutex = Mutex.create();
  let queuedNotificationsMutex = Mutex.create();

  let queuedResponses: ref(list(response)) = ref([]);
  let queuedNotifications: ref(list(notification)) = ref([]);

  let onNotification: Event.t(notification) = Event.create();

  let clearQueuedResponses = () =>
    withMutex(queuedResponseMutex, () => queuedResponses := []);

  let getQueuedResponses = () =>
    withMutex(queuedResponseMutex, () => queuedResponses^);

  let getAndClearNotifications = () =>
    withMutex(
      queuedNotificationsMutex,
      () => {
        let r = queuedNotifications^;
        queuedNotifications := [];
        r;
      },
    );

  let isRunning = ref(true);

  /**
   * NOTE:
   *
   * This handler is called from _another thread_.
   * Use caution when adding logic here, especially
   * anything that touches shared memory
   */
  let handleMessage = (m: M.t) =>
    /* prerr_endline ("Got message: |" ++ M.show(m) ++ "|"); */
    switch (m) {
    | M.List([M.Int(1), M.Int(id), _, v]) =>
      withMutex(queuedResponseMutex, () =>
        queuedResponses :=
          List.append([{responseId: id, payload: v}], queuedResponses^)
      )
    | M.List([M.Int(2), M.String(msg), v]) =>
      withMutex(queuedNotificationsMutex, () =>
        queuedNotifications :=
          List.append(
            [{notificationType: msg, payload: v}],
            queuedNotifications^,
          )
      )
    /* prerr_endline ("Got notification: " ++ msg); */
    | _ => prerr_endline("Unknown message: " ++ M.show(m))
    };

  let dispose1 = Event.subscribe(msgpack.onMessage, m => handleMessage(m));
  /** END NOTE */

  let pump = () =>
    if (isRunning^) {
      let notifications = getAndClearNotifications();

      let f = n => Event.dispatch(onNotification, n);

      /* Because of the way we queue notifications in `handleMessage`,
       * they come in reverse order - therefore, we need to reverse it
       * to get the correct ordering */
      List.rev(notifications) |> List.iter(f);
    };

  let request = (methodName: string, args: M.t) => {
    let requestId = getNextId();

    let request =
      M.List([M.Int(0), M.Int(requestId), M.String(methodName), args]);

    msgpack.write(request);
  };

  let requestSync: requestSyncFunction =
    (methodName: string, args: M.t) => {
      let requestId = getNextId();

      let request =
        M.List([M.Int(0), M.Int(requestId), M.String(methodName), args]);

      clearQueuedResponses();
      msgpack.write(request);

      let hasResponse = v =>
        List.length(List.filter(m => m.responseId == requestId, v)) >= 1;

      Utility.waitForCondition(~timeout=10.0, () =>
        hasResponse(getQueuedResponses())
      );
      let queuedResponses = getQueuedResponses();

      if (!hasResponse(queuedResponses)) {
        let errorMessage =
          "Request timed out: "
          ++ methodName
          ++ " ("
          ++ string_of_int(requestId)
          ++ ")"
          ++ M.show(args);
        prerr_endline(errorMessage);
        prerr_endline("Queued responses: ");
        List.iter(
          m =>
            prerr_endline(
              "id: "
              ++ string_of_int(m.responseId)
              ++ " payload: "
              ++ M.show(m.payload),
            ),
          queuedResponses,
        );
        raise(RequestFailed(errorMessage));
      };

      let matchingResponse =
        queuedResponses
        |> List.filter(m => m.responseId == requestId)
        |> List.hd;

      /* prerr_endline("Got response!"); */
      let ret: M.t = matchingResponse.payload;
      ret;
    };

  let dispose = () => {
    isRunning := false;
    dispose1();
  };

  let ret: t = {pump, request, requestSync, onNotification, dispose};
  ret;
};
