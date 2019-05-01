/*
 * ExtensionHostClient.re
 *
 * This is a client-side API for integrating with our VSCode extension host API.
 *
 */

open Oni_Core;

module Protocol = ExtensionHostProtocol;

module In = Protocol.IncomingNotifications;
module Out = Protocol.OutgoingNotifications;

type t = {transport: ExtensionHostTransport.t};

type simpleCallback = unit => unit;
let defaultCallback: simpleCallback = () => ();

let apply = (f, r) => {
  switch (r) {
  | Some(v) => f(v)
  | None => ()
  };
};

let start =
    (
      ~initData=ExtensionHostInitData.create(),
      ~onInitialized=defaultCallback,
      ~onClosed=defaultCallback,
      ~onStatusBarSetEntry,
      setup: Setup.t,
    ) => {
  let onMessage = (scope, method, args) => {
    switch (scope, method, args) {
    | ("MainThreadStatusBar", "$setEntry", args) =>
      In.StatusBar.parseSetEntry(args) |> apply(onStatusBarSetEntry)
	  Ok(None);
    | _ => Ok(None);
    };
	};

    let transport =
      ExtensionHostTransport.start(
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
  ExtensionHostTransport.send(
    v.transport,
    Out.ExtensionService.activateByEvent(evt),
  );
};

let addDocument = (doc, v) => {
	ExtensionHostTransport.send(
	v.transport,
	Out.DocumentsAndEditors.acceptDocumentsAndEditorsDelta(
		~addedDocuments=[doc],
		~removedDocuments=[],
		(),
	)
	);
};

let updateDocument = (uri, modelChange, dirty, v) => {
	ExtensionHostTransport.send(
	v.transport,
	Out.Documents.acceptModelChanged(uri, modelChange, dirty)
	);
};

let pump = (v: t) => ExtensionHostTransport.pump(v.transport);

let close = (v: t) => ExtensionHostTransport.close(v.transport);
