/*
 * NeovimApi.re
 *
 * Wrapper on-top of MsgPack primitives to handle Neovim's primitives:
 * - notifications
 * - requests
 */

open Rench;

type requestSyncFunction = (methodName: string, args: list(Msgpck.t));

type t = {
    requestSync: requestSyncFunction;
};

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
    let s = Unix.gettimeofday();
    while (!f() && (Unix.gettimeofday() -. s < timeout) {
        Unix.sleepf(0.005);
    };   
};

let make = (msgpack: MsgpackTransport.t) => {

    let queuedResponses: ref(list(response)) = ref([]);
    let queuedNotifications: ref(list(Msgpck.t)) = ref([]);

    let clearQueuedResponses = () => queuedResponses := [];

    let handleMessage = (m: Msgpck.t) => {
        switch (m) {
        | Msgpck.List([Msgpck.Int(1), Msgpck.Int(id), ...m]) => {
            queuedResponses := List.append([{responseId: id, payload: m}], queuedResponses);
            prerr_endline ("Got response: " ++ msg);
        }
        | Msgpck.List([Msgpck.Int(2), Msgpck.String(msg), ...m]) => {
            queuedNotifications := List.append([m], queuedNotifications);
            prerr_endline ("Got notification: " ++ msg);
        }
        | _ => prerr_endline ("Unknown message");
        };
    };

    Event.subscribe(msgpack.onMesssage, (m) => {
        handleMessage(m);
    });

    let requestSync = (methodName: string, args: list(Msgpck.t)) => {
        let requestId = getNextId();

        let request = Msgpck.List([Msgpck.Int(0), Msgpck.Int(requestId), Msgpck.String(methodName), args]);

        clearQueuedResponses();
        msgpack.write(request);

        prerr_endline ("starting request");
        waitForCondition(() => List.length(queuedResponses) >= 1);
        prerr_endline ("ending request");

        let matchingResponse = 
            List.filter((m) => m.responseId == requestId, queuedResponses^) 
            |> List.hd;

        prerr_endline ("Got response!");
        matchingResponse.payload
    };

    let ret: t = {
        requestSync
    };
};
