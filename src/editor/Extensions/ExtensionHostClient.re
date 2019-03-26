/*
 * ExtensionHostClient.re
 *
 * This is a client-side API for integrating with our VSCode extension host API.
 * 
 */

open Oni_Core;
open Reason_jsonrpc;
open Rench;

type t = {
   process: NodeProcess.t,
   rpc: Rpc.t,
}

let getPathToExtensionHostScript = (setup: Setup.t) => {
    Path.join(Setup.getExtensionHostPath(setup), "out/bootstrap-fork.js");
};

let emptyJsonValue = `Assoc([]);

type simpleCallback = unit => unit;
let defaultCallback: simpleCallback = () => ();

let start =
    (
      ~onClosed=defaultCallback,
      setup: Setup.t,
    ) => {

  let args = ["--type=extensionHost"];
  let process = NodeProcess.start(~args, setup, getPathToExtensionHostScript(setup));


  let onNotification = (n: Notification.t, _) => {
    switch (n.method, n.params) {
    | _ => print_endline("[Extension Host Client] Unknown message: " ++ n.method);
    };
  };

  let onRequest = (_, _) => Ok(emptyJsonValue);

  let rpc =
    Rpc.start(
      ~onNotification,
      ~onRequest,
      ~onClose=onClosed,
      process.stdout,
      process.stdin,
    );

  {process, rpc}
};
