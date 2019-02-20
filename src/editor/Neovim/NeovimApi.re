/*
 * NeovimApi.re
 *
 * Wrapper on-top of MsgPack primitives to handle Neovim's primitives:
 * - notifications
 * - requests
 */

open Oni_Core;
open Rench;

type requestSyncFunction = (string, Msgpck.t) => Msgpck.t;

type response = {
  responseId: int,
  payload: Msgpck.t,
};

type notification = {
  notificationType: string,
  payload: Msgpck.t,
};

type t = {
  requestSync: requestSyncFunction,
  /*
   * Pump should be called periodically to dispatch any queued notifications.
   */
  pump: unit => unit,
  onNotification: Event.t(notification),
};

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

  let clearQueuedResponses = () => {
    withMutex(queuedResponseMutex, () => queuedResponses := []);
  };

  let getQueuedResponses = () => {
    withMutex(queuedResponseMutex, () => queuedResponses^);
  };

  let getAndClearNotifications = () => {
    withMutex(
      queuedNotificationsMutex,
      () => {
        let r = queuedNotifications^;
        queuedNotifications := [];
        r;
      },
    );
  };

  /**
   * NOTE:
   *
   * This handler is called from _another thread_.
   * Use caution when adding logic here, especially
   * anything that touches shared memory
   */
  let handleMessage = (m: Msgpck.t) => {
    /* prerr_endline ("Got message: |" ++ Msgpck.show(m) ++ "|"); */
    switch (m) {
    | Msgpck.List([Msgpck.Int(1), Msgpck.Int(id), _, v]) =>
      withMutex(queuedResponseMutex, () =>
        queuedResponses :=
          List.append([{responseId: id, payload: v}], queuedResponses^)
      )
    | Msgpck.List([Msgpck.Int(2), Msgpck.String(msg), v]) =>
      withMutex(queuedNotificationsMutex, () =>
        queuedNotifications :=
          List.append(
            [{notificationType: msg, payload: v}],
            queuedNotifications^,
          )
      )
    /* prerr_endline ("Got notification: " ++ msg); */
    | _ => prerr_endline("Unknown message: " ++ Msgpck.show(m))
    };
  };

  let _ = Event.subscribe(msgpack.onMessage, m => handleMessage(m));
  /** END NOTE */

  let pump = () => {
    let notifications = getAndClearNotifications();

    let f = n => Event.dispatch(onNotification, n);

    /* Because of the way we queue notifications in `handleMessage`, 
     * they come in reverse order - therefore, we need to reverse it 
     * to get the correct ordering */
    List.rev(notifications)
    |> List.iter(f);
  };

  let requestSync: requestSyncFunction =
    (methodName: string, args: Msgpck.t) => {
      let requestId = getNextId();

      let request =
        Msgpck.List([
          Msgpck.Int(0),
          Msgpck.Int(requestId),
          Msgpck.String(methodName),
          args,
        ]);

      clearQueuedResponses();
      msgpack.write(request);

      Utility.waitForCondition(() => List.length(getQueuedResponses()) >= 1);

      let matchingResponse =
        List.filter(m => m.responseId == requestId, queuedResponses^)
        |> List.hd;

      /* prerr_endline("Got response!"); */
      let ret: Msgpck.t = matchingResponse.payload;
      ret;
    };

  let ret: t = {pump, requestSync, onNotification};
  ret;
};
