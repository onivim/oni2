/*
 * ExtensionHostProtocol.re
 *
 * This module documents & types the protocol for communicating with the VSCode Extension Host.
 *
 */

module ModelConfig = Configuration;

open Oni_Core;
open Oni_Core.Utility;

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

module OneBasedPosition = {
  [@deriving (show({with_path: false}), yojson({strict: false}))]
  type t = {
    lineNumber: int,
    column: int,
  };

  let ofPosition = (p: Position.t) => {
    lineNumber: p.line |> Index.toOneBasedInt,
    column: p.character |> Index.toOneBasedInt,
  };

  let ofInt1 = (~lineNumber, ~column, ()) => {lineNumber, column};
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
    let newLines = Array.length(bu.lines);
    let isInsert =
      newLines >= Index.toInt0(bu.endLine) - Index.toInt0(bu.startLine);

    let startLine = Index.toInt0(bu.startLine);
    let endLine = Index.toInt0(bu.endLine);

    let endLine = max(endLine, startLine);
    let endCharacter = 0;

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
    severity: int,
    // TODO:
    // source: string,
    //code: string,
    // relatedInformation: DiagnosticRelatedInformation.t,
  };

  type t = {
    range: OneBasedRange.t,
    message: string,
    severity: int,
    // TODO:
    // source: string,
    // code: string,
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
        severity: ret.severity,
        // TODO:
        //source: ret.source,
        //code: ret.code,
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
      let perFileDiagnostics =
        perFileDiagnostics
        |> List.map(Diagnostics.of_yojson)
        |> List.filter_map(Utility.resultToOption);
      Some({name, perFileDiagnostics});
    | _ => None
    };
  };
};

module SuggestionItem = {
  [@deriving yojson({strict: false})]
  type t = {
    label: string,
    insertText: string,
    kind: option(int),
    detail: option(string),
  };

  let of_yojson = json => {
    Yojson.Safe.Util.(
      try({
        let label = json |> member("label") |> to_string;
        let insertText = json |> member("insertText") |> to_string;
        let kind = json |> member("kind") |> to_int_option;
        let detail = json |> member("detail") |> to_string_option;
        Ok({label, insertText, kind, detail});
      }) {
      | Undefined(msg, _) => Error(msg)
      }
    );
  };
};

module Suggestions = {
  type t = list(SuggestionItem.t);

  let of_yojson = json => {
    switch (json) {
    | `List(suggestions) =>
      let result =
        suggestions
        |> List.map(SuggestionItem.of_yojson)
        |> List.map(resultToOption)
        |> List.filter_map(v => v);
      Ok(result);
    | _ => Error("Unable to parse Suggestions")
    };
  };
};

module Workspace = {
  module Folder = {
    type t = {
      uri: Uri.t,
      name: string,
      id: string,
    };

    let to_yojson: t => Yojson.Safe.t =
      folder => {
        `Assoc([
          ("uri", Uri.to_yojson(folder.uri)),
          ("name", `String(folder.name)),
          ("id", `String(folder.id)),
        ]);
      };
  };

  type t = {
    id: string,
    name: string,
    folders: list(Folder.t),
  };

  let empty: t = {id: "No workspace", name: "No workspace", folders: []};

  let create = (~folders=[], name, ~id) => {id, name, folders};

  let fromUri = (uri, ~name, ~id) => {
    id,
    name,
    folders: [{uri, name, id}],
  };

  let fromPath = path => {
    id: path,
    name: path,
    folders: [{uri: Uri.fromPath(path), name: path, id: path}],
  };

  let to_yojson: t => Yojson.Safe.t =
    ws => {
      let foldersJson =
        ws.folders |> List.map(Folder.to_yojson) |> (v => `List(v));

      `Assoc([
        ("id", `String(ws.id)),
        ("name", `String(ws.name)),
        ("configuration", `Assoc([])),
        ("folders", foldersJson),
      ]);
    };
};

module LF = LanguageFeatures;

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

  module LanguageFeatures = {
    let parseProvideCompletionsResponse = json => {
      switch (Yojson.Safe.Util.member("suggestions", json)) {
      | `List(_) as items =>
        switch (Suggestions.of_yojson(items)) {
        | Ok(v) => Some(v)
        | Error(_) => None
        }
      | _ => None
      };
    };

    let parseRegisterSuggestSupport = json => {
      switch (json) {
      | [`Int(id), documentSelector, `List(_triggerCharacters), `Bool(_)] =>
        // TODO: Finish parsing
        documentSelector
        |> DocumentSelector.of_yojson
        |> Result.to_option
        |> Option.map(selector => {LF.SuggestProvider.create(~selector, id)})
      | _ => None
      };
    };
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
    let initializeConfiguration = (setup: Setup.t) => {
      let keys = [
        "typescript.suggest.enabled",
        "javascript.suggest.enabled",
        "reason_language_server.location",
      ];

      let contents =
        `Assoc([
          (
            "reason_language_server",
            `Assoc([("location", `String(setup.rlsPath))]),
          ),
          (
            "typescript",
            `Assoc([("suggest", `Assoc([("enabled", `Bool(true))]))]),
          ),
          (
            "javascript",
            `Assoc([("suggest", `Assoc([("enabled", `Bool(true))]))]),
          ),
        ]);

      // TODO: Pull from our defaults + user defaults
      let initialConfiguration =
        ModelConfig.create(
          ~defaults=ModelConfig.Model.create(~keys, contents),
          (),
        );

      _buildNotification(
        "ExtHostConfiguration",
        "$initializeConfiguration",
        `List([initialConfiguration |> ModelConfig.to_yojson]),
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

  module LanguageFeatures = {
    let provideCompletionItems =
        (handle: int, resource: Uri.t, position: OneBasedPosition.t) =>
      // TODO: CompletionContext ?
      // TODO: CancelationToken
      _buildNotification(
        "ExtHostLanguageFeatures",
        "$provideCompletionItems",
        `List([
          `Int(handle),
          Uri.to_yojson(resource),
          OneBasedPosition.to_yojson(position),
          `Assoc([]),
        ]),
      );
  };

  module Workspace = {
    [@deriving yojson({strict: false, exn: true})]
    type workspaceInfo = {
      id: string,
      name: string,
      folders: list(string),
    };

    let initializeWorkspace = (workspace: Workspace.t) => {
      _buildNotification(
        "ExtHostWorkspace",
        "$initializeWorkspace",
        `List([Workspace.to_yojson(workspace)]),
      );
    };

    let acceptWorkspaceData = (workspace: Workspace.t) => {
      _buildNotification(
        "ExtHostWorkspace",
        "$acceptWorkspaceData",
        `List([Workspace.to_yojson(workspace)]),
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
