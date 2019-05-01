/*
 * ExtensionHostClient.re
 *
 * This is a client-side API for integrating with our VSCode extension host API.
 *
 */

open Oni_Core;
open Reason_jsonrpc;
open Rench;

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
  let onMessage = json => {
    switch (scope, method, args) {
    | ("MainThreadStatusBar", "$setEntry", args) =>
      In.StatusBar.parseSetEntry(args) |> apply(onStatusBarSetEntry)
    | _ => ()
    };

    let transport =
      ExtensionHostTransport.start(
        ~initData,
        ~onInitialized,
        ~onMessage,
        ~onClosed,
        setup,
      );
    transport;
  };
  ();
};

let ofTransport = (transport: ExtensionHostTransport.t) => {
  transport;
};

let activateByEvent = (evt, v) => {
  ExtensionHostTransport.send(
    v.transport,
    Out.ExtensionService.activateByEvent(evt),
  );
};

let pump = (v: t) => ExtensionHostTransport.pump(v);

let close = (v: t) => ExtensionHostTransport.close(v);
