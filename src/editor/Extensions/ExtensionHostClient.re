/*
 * ExtensionHostClient.re
 *
 * This is a client-side API for integrating with our VSCode extension host API.
 * 
 */

open Oni_Core;
open Reason_jsonrpc;
/* open Revery; */

type t = {
   process: NodeProcess.t,
   rpc: Rpc.t,
}

let emptyJsonValue = `Assoc([]);

type simpleCallback = unit => unit;
let defaultCallback: simpleCallback = () => ();

module Protocol {

    module MessageTypes {
        let initialized = 0;
        let ready = 1;
        let initData = 2;
        let terminate = 3;
    };

    module LogLevel {
        let trace = 0;
        let debug = 1;
        let info = 2;
        let warning = 3;
        let error = 4;
        let critical = 5;
        let off = 6;
    };

    module Environment {
        type t = {
            /* isExtensionDevelopmentDebug: bool, */
            appRootPath: string,
            appSettingsHomePath: string,
            /* extensionDevelopmentLocationPath: string, */
            /* extensionTestsLocationPath: string, */
            globalStorageHomePath: string,
        };

        let create = (~appRootPath, ~appSettingsHomePath, ~globalStorageHomePath, ()) => {
            appRootPath,
            appSettingsHomePath,
            globalStorageHomePath,
        };
    };

/*     module InitData { */
/*         [@deriving (show, yojson({strict: false, exn: true}))] */
/*         type t = { */
/*             parentPid: int, */
/*             environment: Environment.t, */
/*             logLevel: int, */
/*             logsLocationPath: string, */
/*             autoStart: bool, */
/*         }; */
/*     }; */


    module Notification {

        exception NotificationParseException(string);

        type t = {
            msgType: int,
            reqId: int,
            payload: Yojson.Safe.json,
        };

        let of_yojson = (json: Yojson.Safe.json) => {
            switch (json) {
            | (`Assoc([
                ("type", `Int(t)),
                ("reqId", `Int(reqId)),
                ("payload", payload),
            ])) => {
                { msgType: t, reqId, payload} ;
            }
            | _ => raise(NotificationParseException("Unable to parse: " ++ Yojson.Safe.to_string(json)));
            }
        };
    };
}

type messageHandler = (int, Yojson.Safe.json) => result(option(Yojson.Safe.json), string);
let defaultMessageHandler = (_, _) => Ok(None);

let start =
    (
      ~onMessage=defaultMessageHandler,
      ~onClosed=defaultCallback,
      setup: Setup.t,
    ) => {

  let args = ["--type=extensionHost"];
  let env = ["AMD_ENTRYPOINT=vs/workbench/services/extensions/node/extensionHostProcess"];
  let process = NodeProcess.start(~args, ~env, setup, setup.extensionHostPath);

  let handleMessage = (id: int, _reqId: int, payload: Yojson.Safe.json) => {

      switch(onMessage(id, payload)) {
      | Ok(None) => ()
      | Ok(Some(_)) => {
        /* TODO: Send response */
          ();
      }
      | Error(_) => {
        /* TODO: Send error */
          ();
      }
      };
  }

  let onNotification = (n: Notification.t, _) => {
    switch (n.method, n.params) {
    | ("host/msg", json) => {
        open Protocol.Notification;
        print_endline("[Extension Host Client] Unknown message: " ++ n.method);
        print_endline ("JSON: " ++ Yojson.Safe.to_string(json));
        let parsedMessage = Protocol.Notification.of_yojson(json);
        handleMessage(parsedMessage.msgType, parsedMessage.reqId, parsedMessage.payload);
    }
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
