/*
 * ExtensionHostClient.re
 *
 * This is a client-side API for integrating with our VSCode extension host API.
 * 
 */

module StartupInfo {
    type t;
}

open Oni_Core;
open Rench;

let getPathToExtensionHostScript = (setup: Setup.t) => {
    Path.join(Setup.getExtensionHostPath(setup), "out/)
};

let emptyJsonValue = `Assoc([]);

let start =
    (
      setup: Setup.t,
    ) => {

  let args = "[--type=extensionHost"];
  let process = NodeProcess.start(~args, setup, getPathToExtensionHostScript(setup));;


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
};
