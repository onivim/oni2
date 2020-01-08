/*
 * ExtHostTransport.re
 *
 * This is a client-side API for integrating with our VSCode extension host API.
 *
 */

open Oni_Core;
open Reason_jsonrpc;
open Rench;
/* open Revery; */

module Log = (val Log.withNamespace("Oni2.ExtHostTransport"));

module Protocol = ExtHostProtocol;

module Workspace = Protocol.Workspace;

type t = {
  process: NodeProcess.t,
  rpc: Rpc.t,
  send: (MessageType.t, Yojson.Safe.t) => unit,
  sendRequest: (MessageType.t, Yojson.Safe.t) => Lwt.t(Yojson.Safe.t),
};

let emptyJsonValue = `Assoc([]);

type simpleCallback = unit => unit;
let defaultCallback: simpleCallback = () => ();

type messageHandler =
  (string, string, list(Yojson.Safe.t)) =>
  result(option(Yojson.Safe.t), string);
let defaultMessageHandler = (_, _, _) => Ok(None);

let start =
    (
      ~initialConfiguration,
      ~initData,
      ~initialWorkspace=Workspace.empty,
      ~onInitialized=defaultCallback,
      ~onMessage=defaultMessageHandler,
      ~onClosed=defaultCallback,
      setup: Setup.t,
    ) => {
  let args = ["--type=extensionHost"];
  let env = [
    "PATH=" ++ ShellUtility.getShellPath(),
    "AMD_ENTRYPOINT=vs/workbench/services/extensions/node/extensionHostProcess",
    "VSCODE_PARENT_PID=" ++ string_of_int(Process.pid()),
  ];

  let process =
    NodeProcess.start(
      ~args,
      ~env,
      setup,
      Setup.getNodeExtensionHostPath(setup),
    );

  let lastReqId = ref(0);
  let rpcRef = ref(None);
  let initialized = ref(false);
  let queuedCallbacks = ref([]);
  let replyIdToResolver: Hashtbl.t(int, Lwt.u(Yojson.Safe.t)) =
    Hashtbl.create(128);

  let queue = f => {
    queuedCallbacks := [f, ...queuedCallbacks^];
  };

  let send = (msgType: MessageType.t, msg: Yojson.Safe.t) => {
    switch (rpcRef^) {
    | None =>
      Log.error("RPC not initialized.");
      (-1);
    | Some(v) =>
      incr(lastReqId);
      let reqId = lastReqId^;

      let request =
        `Assoc([
          ("type", `Int(msgType |> MessageType.toInt)),
          ("reqId", `Int(reqId)),
          ("payload", msg),
        ]);

      Rpc.sendNotification(v, "ext/msg", request);
      reqId;
    };
  };

  let sendNotification = (msg: Yojson.Safe.t) => {
    let _ = send(MessageType.requestJsonArgs, msg);
    ();
  };

  let sendRequest = (msgType, msg: Yojson.Safe.t) => {
    let reqId = send(msgType, msg);
    let (promise, resolver) = Lwt.task();

    Hashtbl.add(replyIdToResolver, reqId, resolver);

    promise;
  };

  let sendResponse = (msgType, reqId, msg) => {
    switch (rpcRef^) {
    | None => Log.error("RPC not initialized.")
    | Some(v) =>
      let response =
        `Assoc([
          ("type", `Int(msgType)),
          ("reqId", `Int(reqId)),
          ("payload", msg),
        ]);
      Rpc.sendNotification(v, "ext/msg", response);
    };
  };

  let handleMessage = (reqId: int, payload: Yojson.Safe.t) =>
    switch (payload) {
    | `Assoc([
        ("rpcName", `String(scopeName)),
        ("methodName", `String(methodName)),
        ("args", `List(args)),
      ]) =>
      switch (onMessage(scopeName, methodName, args)) {
      | Ok(None) => sendResponse(12, reqId, `Assoc([]))
      | _ => sendResponse(12, reqId, `Assoc([]))
      }
    | _ =>
      Log.errorf(m =>
        m("Unknown message: %s", Yojson.Safe.to_string(payload))
      )
    };

  let handleReply = (reqId: int, payload: Yojson.Safe.t) => {
    Log.debugf(m =>
      m("Reply ID: %d payload: %s\n", reqId, Yojson.Safe.to_string(payload))
    );
    switch (Hashtbl.find_opt(replyIdToResolver, reqId)) {
    | Some(resolver) =>
      Lwt.wakeup(resolver, payload);
      Hashtbl.remove(replyIdToResolver, reqId);
    | None => Log.warn("Unmatched reply: " ++ string_of_int(reqId))
    };
  };

  let _sendInitData = () => {
    let _: int =
      send(MessageType.initData, ExtHostInitData.to_yojson(initData));
    ();
  };

  let _handleInitialization = () => {
    onInitialized();
    /* Send workspace and configuration info to get the extensions started */
    open ExtHostProtocol.OutgoingNotifications;

    Configuration.initializeConfiguration(initialConfiguration)
    |> sendNotification;
    Workspace.initializeWorkspace(initialWorkspace) |> sendNotification;

    initialized := true;

    queuedCallbacks^ |> List.rev |> List.iter(f => f());
  };

  let onNotification = (n: Notification.t, _) => {
    switch (n.method, n.params) {
    | ("host/msg", json) =>
      Protocol.Notification.(
        switch (parse(json)) {
        | Request(req) => handleMessage(req.reqId, req.payload)
        | Reply(reply) => handleReply(reply.reqId, reply.payload)
        | Ack(_) => ()
        | Error => ()
        | Ready => _sendInitData()
        | Initialized => _handleInitialization()
        }
      )

    | _ => Log.error("Unknown message: " ++ n.method)
    };
  };

  let onRequest = (_, _) => Ok(emptyJsonValue);

  let rpc =
    Rpc.start(
      ~onNotification,
      ~onRequest,
      ~onClose=onClosed,
      ~scheduler=Revery.App.runOnMainThread,
      process.stdout,
      process.stdin,
    );

  rpcRef := Some(rpc);

  let wrappedSend = (msgType, msg) => {
    let f = () => {
      let _: int = send(msgType, msg);
      ();
    };
    initialized^ ? f() : queue(f);
  };

  {process, rpc, send: wrappedSend, sendRequest};
};

let send = (~msgType=MessageType.requestJsonArgs, v: t, msg: Yojson.Safe.t) => {
  let _ = v.send(msgType, msg);
  ();
};

let request =
    (~msgType=MessageType.requestJsonArgs, v: t, msg: Yojson.Safe.t, f) => {
  let promise = v.sendRequest(msgType, msg);
  let wrapper = json =>
    try(Lwt.return(f(json))) {
    | e =>
      Log.warnf(m =>
        m("Request failed with error: %s", Printexc.to_string(e))
      );
      Lwt.fail(e);
    };
  Lwt.bind(promise, wrapper);
};

let close = (v: t) => {
  v.send(MessageType.terminate, `Assoc([]));
};
