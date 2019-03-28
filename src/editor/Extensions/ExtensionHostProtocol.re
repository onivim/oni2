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
  let requestJsonArgs = 4;
  let acknowledged = 8;
  let replyOkJson = 12;
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

  let create = (~globalStorageHomePath, ()) => {
    globalStorageHomePath;
  };
};


module OutgoingNotifications = {

    let _buildNotification = (scopeName, methodName, payload) => {
        `List([`String(scopeName), `String(methodName), payload]);
    }
   
    module Configuration = {

        let initializeConfiguration = () => {
            _buildNotification("ExtHostConfiguration", "$initializeConfiguration", `List([`Assoc([])]));
        };
    };

    module  Workspace = {

        [@deriving (show({with_path: false}), yojson({strict: false, exn: true}))]
        type workspaceInfo = {
            id: string,
            name: string,
            folders: list(string),
        };

        let initializeWorkspace = (id: string, name: string) => {
            let wsinfo = {
                id,
                name,
                folders: [],
            }; 

            _buildNotification("ExtHostWorkspace", "$initializeWorkspace", `List([
                workspaceInfo_to_yojson(wsinfo)
            ]));
        }
    }
};

module Notification = {
  exception NotificationParseException(string);

  type requestOrReply = {
    msgType: int,
    reqId: int,
    payload: Yojson.Safe.json,
  };

  type ack = {
      msgType: int,
      reqId: int,
  };

  type t = 
  | Request(requestOrReply)
  | Reply(requestOrReply)
  | Ack(ack);

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

  let parse = (json: Yojson.Safe.json) => {
    switch(json) {
    | `Assoc([("type", MessageType.replyOkJson), ..._]) => {
       Reply(of_yojson(json))
    }
    | `Assoc([("type", MessageType.requestJsonArgs), ..._]) => {
       Request(of_yojson(json))
    }
    | `Assoc([("type", MessageType.ack), ..._]) => Ack(ack_of_yojson(json));
    | _ => raise(NotificationParseException("Unknown message: " ++ Yojson.Safe.to_string(json)))
    };
  };
};
