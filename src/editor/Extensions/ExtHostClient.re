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

let addDocument = (doc, v) => {
	ExtHostTransport.send(
	v.transport,
	Out.DocumentsAndEditors.acceptDocumentsAndEditorsDelta(
		~addedDocuments=[doc],
		~removedDocuments=[],
		(),
	)
	);
};

let updateDocument = (uri, modelChange, dirty, v) => {
	ExtHostTransport.send(
	v.transport,
	Out.Documents.acceptModelChanged(uri, modelChange, dirty)
	);
};

let pump = (v: t) => ExtHostTransport.pump(v.transport);

let close = (v: t) => ExtHostTransport.close(v.transport);
