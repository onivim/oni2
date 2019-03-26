/*
 * ExtensionHostClient.re
 *
 * This is a client-side API for integrating with our VSCode extension host API.
 * 
 */

open Oni_Core;
open Reason_jsonrpc;

type t = {
   process: NodeProcess.t,
   rpc: Rpc.t,
}

let emptyJsonValue = `Assoc([]);

type simpleCallback = unit => unit;
let defaultCallback: simpleCallback = () => ();

let start =
    (
      ~onClosed=defaultCallback,
      setup: Setup.t,
    ) => {

  let args = ["--type=extensionHost"];
  let env = ["AMD_ENTRYPOINT=vs/workbench/services/extensions/node/extensionHostProcess"];
  let process = NodeProcess.start(~args, ~env, setup, setup.extensionHostPath);

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

let pump = (v: t) => Rpc.pump(v.rpc);
