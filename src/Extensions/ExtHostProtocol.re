/*
 * ExtensionHostProtocol.re
 *
 * This module documents & types the protocol for communicating with the VSCode Extension Host.
 *
 */

open Oni_Core;
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

module Eol = {
  [@deriving (show({with_path: false}), yojson({strict: false}))]
  type t =
    | [@name "\n"] LF
    | [@name "\r\n"] CRLF;

  let default = Sys.win32 ? CRLF : LF;

  let toString = (v: t) =>
    switch (v) {
    | CRLF => "\r\n"
    | LF => "\n"
    };
};

module PackedString = {
  [@deriving (show({with_path: false}), yojson({strict: false, exn: true}))]
  type json = {
    value: string,
    _lower: string,
  };

  let parse = (v: Yojson.Safe.t) => {
    json_of_yojson_exn(v) |> (r => r.value);
  };
};

module ModelAddedDelta = {
  [@deriving yojson({strict: false})]
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

  let create =
      (~startLineNumber, ~endLineNumber, ~startColumn, ~endColumn, ()) => {
    startLineNumber,
    endLineNumber,
    startColumn,
    endColumn,
  };

  let ofRange = (r: Range.t) => {
    startLineNumber: r.startPosition.line |> Index.toOneBasedInt,
    endLineNumber: r.endPosition.line |> Index.toOneBasedInt,
    startColumn: r.startPosition.character |> Index.toOneBasedInt,
    endColumn: r.endPosition.character |> Index.toOneBasedInt,
  };

  let toRange = (v: t) => {
    Range.ofInt1(
      ~startLine=v.startLineNumber,
      ~endLine=v.endLineNumber,
      ~startCharacter=v.startColumn,
      ~endCharacter=v.endColumn,
      (),
    );
  };
};

module ModelContentChange = {
  [@deriving (show({with_path: false}), yojson({strict: false}))]
  type t = {
    range: OneBasedRange.t,
    text: string,
  };

  let create = (~range: Range.t, ~text: string, ()) => {
    range: OneBasedRange.ofRange(range),
    text,
  };

  let joinLines = (separator: string, lines: list(string)) => {
    String.concat(separator, lines);
  };

  let getRangeFromEdit = (bu: BufferUpdate.t) => {
    let isInsert =
      Index.toZeroBasedInt(bu.endLine) == Index.toZeroBasedInt(bu.startLine);

    let startLine = Index.toZeroBasedInt(bu.startLine);
    let endLine = Index.toZeroBasedInt(bu.endLine) - 1;

    let endLine = max(endLine, startLine);
    let endCharacter = isInsert ? 0 : 2147483647;

    let range =
      Range.create(
        ~startLine=ZeroBasedIndex(startLine),
        ~endLine=ZeroBasedIndex(endLine),
        ~startCharacter=ZeroBasedIndex(0),
        ~endCharacter=ZeroBasedIndex(endCharacter),
        (),
      );

    (isInsert, range);
  };

  let ofBufferUpdate = (bu: BufferUpdate.t, eol: Eol.t) => {
    let (isInsert, range) = getRangeFromEdit(bu);
    let text = joinLines(Eol.toString(eol), bu.lines |> Array.to_list);

    let text = isInsert ? text ++ Eol.toString(eol) : text;

    {range: OneBasedRange.ofRange(range), text};
  };
};

module ModelChangedEvent = {
  [@deriving (show({with_path: false}), yojson({strict: false}))]
  type t = {
    changes: list(ModelContentChange.t),
    eol: Eol.t,
    versionId: int,
  };

  let create = (~changes, ~eol, ~versionId, ()) => {changes, eol, versionId};
};

module Diagnostic = {
  [@deriving yojson({strict: false})]
  type json = {
    startLineNumber: int,
    endLineNumber: int,
    startColumn: int,
    endColumn: int,
    message: string,
    source: string,
    code: string,
    severity: int,
    // TODO:
    // relatedInformation: DiagnosticRelatedInformation.t,
  };

  type t = {
    range: OneBasedRange.t,
    message: string,
    source: string,
    code: string,
    severity: int,
    // TODO:
    // relatedInformation: DiagnosticRelatedInformation.t,
  };

  let of_yojson = json => {
    switch (json_of_yojson(json)) {
    | Ok(ret) =>
      let range =
        OneBasedRange.create(
          ~startLineNumber=ret.startLineNumber,
          ~startColumn=ret.startColumn,
          ~endLineNumber=ret.endLineNumber,
          ~endColumn=ret.endColumn,
          (),
        );

      Ok({
        range,
        message: ret.message,
        source: ret.source,
        code: ret.code,
        severity: ret.severity,
      });
    | Error(msg) => Error(msg)
    };
  };

  let to_yojson = _ => {
    // TODO:
    failwith("Not used");
  };
};

module Diagnostics = {
  [@deriving yojson({strict: false})]
  type t = (Uri.t, list(Diagnostic.t));
};

module DiagnosticsCollection = {
  type t = {
    name: string,
    perFileDiagnostics: list(Diagnostics.t),
  };

  let of_yojson = (json: Yojson.Safe.t) => {
    switch (json) {
    | `List([`String(name), `List(perFileDiagnostics)]) =>
      print_endline(
        "DIAG BEFORE PARSING for "
        ++ name
        ++ string_of_int(List.length(perFileDiagnostics)),
      );
      let perFileDiagnostics =
        List.map(Diagnostics.of_yojson, perFileDiagnostics);

      List.iter(
        v =>
          switch (v) {
          | Ok(_) => ()
          | Error(msg) => print_endline("ERROR: " ++ msg)
          },
        perFileDiagnostics,
      );

      let perFileDiagnostics =
        perFileDiagnostics |> Utility.filterMap(Utility.resultToOption);
      print_endline(
        "DIAG AFTER PARSING for "
        ++ name
        ++ string_of_int(List.length(perFileDiagnostics)),
      );
      Some({name, perFileDiagnostics});
    | _ => None
    };
  };
};

module IncomingNotifications = {
  module StatusBar = {
    let parseSetEntry = args => {
      switch (args) {
      | [
          `Int(id),
          _,
          `String(text),
          _,
          _,
          _,
          `Int(alignment),
          `Int(priority),
        ] =>
        Some((id, text, alignment, priority))
      | _ => None
      };
    };
  };

  module Diagnostics = {
    let parseClear = args =>
      switch (args) {
      | [`String(owner)] => Some(owner)
      | _ => None
      };

    let parseChangeMany = args =>
      DiagnosticsCollection.of_yojson(`List(args));
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
      [@deriving yojson({strict: false})]
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

  module ExtensionService = {
    let activateByEvent = (event: string) => {
      _buildNotification(
        "ExtHostExtensionService",
        "$activateByEvent",
        `List([`String(event)]),
      );
    };
  };

  module Workspace = {
    [@deriving yojson({strict: false, exn: true})]
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
    payload: Yojson.Safe.t,
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

  let of_yojson = (json: Yojson.Safe.t) => {
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

  let parse = (json: Yojson.Safe.t) => {
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
