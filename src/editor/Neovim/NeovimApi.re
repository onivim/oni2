/*
 * NeovimApi.re
 *
 * Wrapper on-top of MsgPack primitives to handle Neovim's primitives:
 * - notifications
 * - requests
 */

open Rench;

type requestSyncFunction = (string, Msgpck.t) => Msgpck.t;

type t = {requestSync: requestSyncFunction};

let currentId = ref(0);

let getNextId = () => {
  let ret = currentId^;
  currentId := currentId^ + 1;
  ret;
};

type response = {
  responseId: int,
  payload: Msgpck.t,
};

let waitForCondition = (~timeout=1.0, f) => {
  let thread = Thread.create(() => {
      let s = Unix.gettimeofday();
      while (!f() && Unix.gettimeofday() -. s < timeout) {
        Unix.sleepf(0.002);
        /* Unix.sleepf(0.002); */
      };
  }, ());

  Thread.join(thread);
};

let make = (msgpack: MsgpackTransport.t) => {
  let queuedResponses: ref(list(response)) = ref([]);
  let queuedNotifications: ref(list(Msgpck.t)) = ref([]);

  let clearQueuedResponses = () => {
    queuedResponses := [];
  };

  let handleMessage = (m: Msgpck.t) => {
    prerr_endline ("Got message: |" ++ Msgpck.show(m) ++ "|");
    switch (m) {
    | Msgpck.List([Msgpck.Int(1), Msgpck.Int(id), _, v]) =>
      queuedResponses :=
        List.append([{responseId: id, payload: v}], queuedResponses^)
    | Msgpck.List([Msgpck.Int(2), Msgpck.String(_msg), v, _]) =>
      queuedNotifications := List.append([v], queuedNotifications^)
    /* prerr_endline ("Got notification: " ++ msg); */
    | _ => prerr_endline("Unknown message: " ++ Msgpck.show(m))
    };
  };

  let _ = Event.subscribe(msgpack.onMessage, m => handleMessage(m));

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

      let startTime = Unix.gettimeofday();
      prerr_endline("starting request: " ++ string_of_float(startTime));
      waitForCondition(() => List.length(queuedResponses^) >= 1);
      let endTime = Unix.gettimeofday();
      prerr_endline(
        "ending request: "
        ++ string_of_float(endTime)
        ++ "|"
        ++ string_of_float(endTime -. startTime),
      );

      let matchingResponse =
        List.filter(m => m.responseId == requestId, queuedResponses^)
        |> List.hd;

      prerr_endline("Got response!");
      let ret: Msgpck.t = matchingResponse.payload;
      ret;
    };

  let ret: t = {requestSync: requestSync};
  ret;
};
