/*
 * ExtensionHostProtocol.re
 *
 * This module documents & types the protocol for communicating with the VSCode Extension Host.
 *
 */

module MessageType = {
let initialized = 0;
let ready = 1;
let initData = 2;
let terminate = 3;
};

module LogLevel = {
let trace = 0;
let debug = 1;
let info = 2;
let warning = 3;
let error = 4;
let critical = 5;
let off = 6;
};

module Environment = {
type t = {
  /* isExtensionDevelopmentDebug: bool, */
  /* appRootPath: string, */
  /* appSettingsHomePath: string, */
  /* extensionDevelopmentLocationPath: string, */
  /* extensionTestsLocationPath: string, */
  globalStorageHomePath: string,
};

let create =
    (~globalStorageHomePath, ()) => {
  globalStorageHomePath,
};
};

  module Notification = {
    exception NotificationParseException(string);

    type t = {
      msgType: int,
      reqId: int,
      payload: Yojson.Safe.json,
    };

    let of_yojson = (json: Yojson.Safe.json) => {
      switch (json) {
      | `Assoc([
          ("type", `Int(t)),
          ("reqId", `Int(reqId)),
          ("payload", payload),
        ]) => {
          msgType: t,
          reqId,
          payload,
        }
      | _ =>
        raise(
          NotificationParseException(
            "Unable to parse: " ++ Yojson.Safe.to_string(json),
          ),
        )
      };
    };
  };
