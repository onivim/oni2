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
   * Pump should be called periodically to dispatch any queued notifications.
   */
  pump: unit => unit,
  onNotification: Event.t(notification),
};

exception RequestFailed;

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

  let _ = Event.subscribe(msgpack.onMessage, m => handleMessage(m));
  /** END NOTE */

  let pump = () => {
    let notifications = getAndClearNotifications();

    let f = n => Event.dispatch(onNotification, n);

    /* Because of the way we queue notifications in `handleMessage`,
     * they come in reverse order - therefore, we need to reverse it
     * to get the correct ordering */
    List.rev(notifications) |> List.iter(f);
  };

  let requestSync: requestSyncFunction =
    (methodName: string, args: M.t) => {
      let requestId = getNextId();

      let request =
        M.List([M.Int(0), M.Int(requestId), M.String(methodName), args]);

      clearQueuedResponses();
      msgpack.write(request);

      Utility.waitForCondition(~timeout=10.0, () =>
        List.length(getQueuedResponses()) >= 1
      );

      if (List.length(getQueuedResponses()) == 0) {
        prerr_endline(
          "Request timed out: "
          ++ methodName
          ++ " ("
          ++ string_of_int(requestId)
          ++ ")",
        );
        raise(RequestFailed);
      };

      let matchingResponse =
        List.filter(m => m.responseId == requestId, queuedResponses^)
        |> List.hd;

      /* prerr_endline("Got response!"); */
      let ret: M.t = matchingResponse.payload;
      ret;
    };

  let ret: t = {pump, requestSync, onNotification};
  ret;
};
