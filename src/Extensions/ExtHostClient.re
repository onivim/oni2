/*
 * ExtHostClient.re
 *
 * This is a client-side API for integrating with our VSCode extension host API.
 *
 */

open Oni_Core;

module Protocol = ExtHostProtocol;

module In = Protocol.IncomingNotifications;
module Out = Protocol.OutgoingNotifications;

type t = {transport: ExtHostTransport.t};

type simpleCallback = unit => unit;
let defaultCallback: simpleCallback = () => ();
let defaultOneArgCallback = _ => ();

let apply = (f, r) => {
  switch (r) {
  | Some(v) => f(v)
  | None => ()
  };
};

let start =
    (
      ~initData=ExtHostInitData.create(),
      ~onInitialized=defaultCallback,
      ~onClosed=defaultCallback,
      ~onDiagnosticsChangeMany=defaultOneArgCallback,
      ~onDiagnosticsClear=defaultOneArgCallback,
      ~onDidActivateExtension=defaultOneArgCallback,
      ~onExtensionActivationFailed=defaultOneArgCallback,
      ~onTelemetry=defaultOneArgCallback,
      ~onOutput=defaultOneArgCallback,
      ~onRegisterCommand=defaultOneArgCallback,
      ~onShowMessage=defaultOneArgCallback,
      ~onStatusBarSetEntry,
      setup: Setup.t,
    ) => {
  let onMessage = (scope, method, args) => {
    switch (scope, method, args) {
    | ("MainThreadOutputService", "$register", _) =>
      // TODO: No-op
      Ok(None)
    | ("MainThreadOutputService", "$append", [_, `String(msg)]) =>
      onOutput(msg);
      Ok(None);
    | ("MainThreadDiagnostics", "$changeMany", args) =>
      In.Diagnostics.parseChangeMany(args) |> apply(onDiagnosticsChangeMany);
      Ok(None);
    | ("MainThreadDiagnostics", "$clear", args) =>
      In.Diagnostics.parseClear(args) |> apply(onDiagnosticsClear);
      Ok(None);
    | ("MainThreadTelemetry", "$publicLog", [`String(eventName), json]) =>
      onTelemetry(eventName ++ ":" ++ Yojson.Safe.to_string(json));
      Ok(None);
    | ("MainThreadMessageService", "$showMessage", [_, `String(s), ..._]) =>
      onShowMessage(s);
      Ok(None);
    | ("MainThreadExtensionService", "$onDidActivateExtension", [v, ..._]) =>
      let id = Protocol.PackedString.parse(v);
      onDidActivateExtension(id);
      Ok(None);
    | (
        "MainThreadExtensionService",
        "$onExtensionActivationFailed",
        [v, ..._],
      ) =>
      let id = Protocol.PackedString.parse(v);
      onExtensionActivationFailed(id);
      Ok(None);
    | ("MainThreadCommands", "$registerCommand", [`String(v), ..._]) =>
      onRegisterCommand(v);
      Ok(None);
    | ("MainThreadStatusBar", "$setEntry", args) =>
      In.StatusBar.parseSetEntry(args) |> apply(onStatusBarSetEntry);
      Ok(None);
    | (s, m, _a) =>
      Log.error("Unhandled message - " ++ s ++ ":" ++ m ++ " | ");
      Ok(None);
    };
  };

  let transport =
    ExtHostTransport.start(
      ~initData,
      ~onInitialized,
      ~onMessage,
      ~onClosed,
      setup,
    );
  let ret: t = {transport: transport};
  ret;
};

let activateByEvent = (evt, v) => {
  ExtHostTransport.send(
    v.transport,
    Out.ExtensionService.activateByEvent(evt),
  );
};

let executeContributedCommand = (cmd, v) => {
  ExtHostTransport.send(
    v.transport,
    Out.Commands.executeContributedCommand(cmd),
  );
};

let addDocument = (doc, v) => {
  ExtHostTransport.send(
    v.transport,
    Out.DocumentsAndEditors.acceptDocumentsAndEditorsDelta(
      ~addedDocuments=[doc],
      ~removedDocuments=[],
      (),
    ),
  );
};

let updateDocument = (uri, modelChange, dirty, v) => {
  ExtHostTransport.send(
    v.transport,
    Out.Documents.acceptModelChanged(uri, modelChange, dirty),
  );
};

let pump = (v: t) => ExtHostTransport.pump(v.transport);

let close = (v: t) => ExtHostTransport.close(v.transport);
