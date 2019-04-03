/*
 * ExtensionHostProtocol.re
 *
 * This module documents & types the protocol for communicating with the VSCode Extension Host.
 *
 */

open Oni_Core.Types;

module MessageType = {
  let initialized = 0;
  let ready = 1;
  let initData = 2;
  let terminate = 3;
  let requestJsonArgs = 4;
  let acknowledged = 8;
  let replyOkJson = 12;
  let replyErrError = 13;
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

module Uri = {
  module Scheme = {
    [@deriving (show({with_path: false}), yojson({strict: false}))]
    type t =
      | [@name "file"] File;

    let toString = (v: t) =>
      switch (v) {
      | File => "file"
      };
  };

  [@deriving (show({with_path: false}), yojson({strict: false}))]
  type t = {
    scheme: Scheme.t,
    path: string,
  };

  let createFromFilePath = (path: string) => {scheme: Scheme.File, path};
};

module Eol = {
  [@deriving (show({with_path: false}), yojson({strict: false}))]
  type t =
    | [@name "\n"] LF
    | [@name "\r\n"] CRLF;

  let default = Sys.win32 ? CRLF : LF;
};

module ModelAddedDelta = {
  [@deriving (show({with_path: false}), yojson({strict: false}))]
  type t = {
    uri: Uri.t,
    versionId: int,
    lines: list(string),
    [@key "EOL"]
    eol: Eol.t,
    modeId: string,
    isDirty: bool,
  };

  let create =
      (
        ~uri,
        ~versionId=0,
        ~lines=[],
        ~eol=Eol.default,
        ~modeId,
        ~isDirty=false,
        (),
      ) => {
    uri,
    versionId,
    lines,
    eol,
    modeId,
    isDirty,
  };
};

module OneBasedRange = {
  [@deriving (show({with_path: false}), yojson({strict: false}))]
  type t = {
    startLineNumber: int,
    endLineNumber: int,
    startColumn: int,
    endColumn: int,
  };

  let ofRange = (r: Range.t) => {
    startLineNumber: r.startPosition.line |> Index.toOneBasedInt,
    endLineNumber: r.endPosition.line |> Index.toOneBasedInt,
    startColumn: r.startPosition.character |> Index.toOneBasedInt,
    endColumn: r.endPosition.character |> Index.toOneBasedInt,
  };
};

module ModelContentChange = {
  [@deriving (show({with_path: false}), yojson({strict: false}))]
  type t = {
    range: OneBasedRange.t,
    text: string,
  };
};

module ModelChangedEvent = {
  [@deriving (show({with_path: false}), yojson({strict: false}))]
  type t = {
    changes: list(ModelContentChange.t),
    eol: Eol.t,
    versionId: int,
  };
};

module OutgoingNotifications = {
  let _buildNotification = (scopeName, methodName, payload) => {
    `List([`String(scopeName), `String(methodName), payload]);
  };

  module Commands = {
    let executeContributedCommand = cmd => {
      _buildNotification(
        "ExtHostCommands",
        "$executeContributedCommand",
        `List([`String(cmd)]),
      );
    };
  };

  module Configuration = {
    let initializeConfiguration = () => {
      _buildNotification(
        "ExtHostConfiguration",
        "$initializeConfiguration",
        `List([`Assoc([])]),
      );
    };
  };

  module Documents = {
    let acceptModelChanged =
        (uri: Uri.t, modelChangedEvent: ModelChangedEvent.t, isDirty: bool) => {
      _buildNotification(
        "ExtHostDocuments",
        "$acceptModelChanged",
        `List([
          Uri.to_yojson(uri),
          ModelChangedEvent.to_yojson(modelChangedEvent),
          `Bool(isDirty),
        ]),
      );
    };
  };

  module DocumentsAndEditors = {
    module DocumentsAndEditorsDelta = {
      [@deriving (show({with_path: false}), yojson({strict: false}))]
      type t = {
        removedDocuments: list(Uri.t),
        addedDocuments: list(ModelAddedDelta.t),
        removedEditors: list(string),
        addedEditors: list(string),
      };

      let create = (~removedDocuments, ~addedDocuments, ()) => {
        removedDocuments,
        addedDocuments,
        removedEditors: [],
        addedEditors: [],
      };
    };

    let acceptDocumentsAndEditorsDelta =
        (
          ~removedDocuments: list(Uri.t),
          ~addedDocuments: list(ModelAddedDelta.t),
          (),
        ) => {
      let delta =
        DocumentsAndEditorsDelta.create(
          ~removedDocuments,
          ~addedDocuments,
          (),
        );

      _buildNotification(
        "ExtHostDocumentsAndEditors",
        "$acceptDocumentsAndEditorsDelta",
        `List([DocumentsAndEditorsDelta.to_yojson(delta)]),
      );
    };
  };

  module Workspace = {
    [@deriving
      (show({with_path: false}), yojson({strict: false, exn: true}))
    ]
    type workspaceInfo = {
      id: string,
      name: string,
      folders: list(string),
    };

    let initializeWorkspace = (id: string, name: string) => {
      let wsinfo = {id, name, folders: []};

      _buildNotification(
        "ExtHostWorkspace",
        "$initializeWorkspace",
        `List([workspaceInfo_to_yojson(wsinfo)]),
      );
    };
  };
};

module Notification = {
  exception NotificationParseException(string);

  type requestOrReply = {
    msgType: int,
    reqId: int,
    payload: Yojson.Safe.json,
  };

  [@deriving (show({with_path: false}), yojson({strict: false, exn: true}))]
  type ack = {
    [@key "type"]
    msgType: int,
    reqId: int,
  };

  type t =
    | Initialized
    | Ready
    | Request(requestOrReply)
    | Reply(requestOrReply)
    | Error
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
    | `Assoc([("type", `Int(t)), ("reqId", `Int(reqId))]) => {
        msgType: t,
        reqId,
        payload: `Assoc([]),
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
    switch (json) {
    | `Assoc([("type", `Int(0)), ..._]) => Initialized
    | `Assoc([("type", `Int(1)), ..._]) => Ready
    | `Assoc([("type", `Int(4)), ..._]) => Request(of_yojson(json))
    | `Assoc([("type", `Int(8)), ..._]) => Ack(ack_of_yojson_exn(json))
    | `Assoc([("type", `Int(12)), ..._]) => Reply(of_yojson(json))
    | `Assoc([("type", `Int(13)), ..._]) => Error
    | _ =>
      raise(
        NotificationParseException(
          "Unknown message: " ++ Yojson.Safe.to_string(json),
        ),
      )
    };
  };
};
