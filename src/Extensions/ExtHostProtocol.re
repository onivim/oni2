/*
 * ExtensionHostProtocol.re
 *
 * This module documents & types the protocol for communicating with the VSCode Extension Host.
 *
 */

module ExtConfiguration = Exthost.Configuration;

open EditorCoreTypes;
open Oni_Core;

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

  let ofPosition = (p: Location.t) => {
    lineNumber: p.line |> Index.toOneBased,
    column: p.column |> Index.toOneBased,
  };

  let ofInt1 = (~lineNumber, ~column, ()) => {lineNumber, column};
};

module OneBasedRange = {
  [@deriving (show({with_path: false}), yojson({strict: false, exn: true}))]
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
    startLineNumber: r.start.line |> Index.toOneBased,
    endLineNumber: r.stop.line |> Index.toOneBased,
    startColumn: r.start.column |> Index.toOneBased,
    endColumn: r.stop.column |> Index.toOneBased,
  };

  let toRange = ({startLineNumber, endLineNumber, startColumn, endColumn}) => {
    Range.{
      start:
        Location.{
          line: Index.fromOneBased(startLineNumber),
          column: Index.fromOneBased(startColumn),
        },
      stop:
        Location.{
          line: Index.fromOneBased(endLineNumber),
          column: Index.fromOneBased(endColumn),
        },
    };
  };
};

module DefinitionLink = {
  [@deriving yojson({strict: false, exn: true})]
  type t = {
    [@default None]
    originSelectionRange: option(OneBasedRange.t),
    uri: Uri.t,
    range: OneBasedRange.t,
    /*[@default None]
      targetSelectionRange: option(OneBasedRange.t),*/
  };
};

module DocumentHighlight = {
  [@deriving yojson({strict: false, exn: true})]
  type t = {
    // TODO: kind
    range: OneBasedRange.t,
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
      newLines >= Index.toZeroBased(bu.endLine)
      - Index.toZeroBased(bu.startLine);

    let startLine = Index.toZeroBased(bu.startLine);
    let endLine = Index.toZeroBased(bu.endLine) |> max(startLine);

    let range =
      Range.{
        start:
          Location.{
            line: Index.fromZeroBased(startLine),
            column: Index.zero,
          },
        stop:
          Location.{line: Index.fromZeroBased(endLine), column: Index.zero},
      };

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
        |> List.filter_map(Stdlib.Result.to_option);
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
        |> List.map(Stdlib.Result.to_option)
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

module SuggestProvider = {
  type t = {
    selector: DocumentSelector.t,
    id: int,
  };

  let create = (~selector, id) => {selector, id};
};

/*module DocumentSymbolProvider = {
    type t = {
      selector: DocumentSelector.t,
      id: int,
      label: string,
    };

    let create = (~selector, ~label, id) => {selector, label, id};
  };*/

module BasicProvider = {
  type t = {
    selector: DocumentSelector.t,
    id: int,
  };

  let create = (~selector, id) => {selector, id};
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

    let parseBasicProvider = json => {
      switch (json) {
      | [`Int(id), documentSelector] =>
        documentSelector
        |> DocumentSelector.of_yojson
        |> Stdlib.Result.to_option
        |> Option.map(selector => {BasicProvider.create(~selector, id)})
      | _ => None
      };
    };

    /*let parseDocumentSymbolProvider = json => {
        switch (json) {
        | [`Int(id), documentSelector, `String(label)] =>
          documentSelector
          |> DocumentSelector.of_yojson
          |> Stdlib.Result.to_option
          |> Option.map(selector => {
               DocumentSymbolProvider.create(~selector, ~label, id)
             })
        | _ => None
        };
      };*/

    let parseRegisterSuggestSupport = json => {
      switch (json) {
      | [`Int(id), documentSelector, `List(_triggerCharacters), `Bool(_)] =>
        documentSelector
        |> DocumentSelector.of_yojson
        |> Stdlib.Result.to_option
        |> Option.map(selector => {SuggestProvider.create(~selector, id)})
      // TODO: Finish parsing
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
    let executeContributedCommand = (cmd, arguments) => {
      _buildNotification(
        "ExtHostCommands",
        "$executeContributedCommand",
        `List([`String(cmd), ...arguments]),
      );
    };
  };

  module Configuration = {
    let initializeConfiguration = config =>
      _buildNotification(
        "ExtHostConfiguration",
        "$initializeConfiguration",
        `List([config |> ExtConfiguration.to_yojson]),
      );

    let acceptConfigurationChanged = (config, changed) =>
      _buildNotification(
        "ExtHostConfiguration",
        "$acceptConfigurationChanged",
        `List([
          config |> ExtConfiguration.to_yojson,
          `Assoc([
            (
              "changedConfiguration",
              changed |> ExtConfiguration.Model.to_yojson,
            ),
            ("changedConfigurationByResource", `Assoc([])),
          ]),
        ]),
      );
  };

  module Decorations = {
    let provideDecorations = (handle: int, uri: Uri.t) =>
      _buildNotification(
        "ExtHostDecorations",
        "$provideDecorations",
        `List([
          `List([
            `Assoc([
              ("id", `Int(0)),
              ("handle", `Int(handle)),
              ("uri", Uri.to_yojson(uri)),
            ]),
          ]),
        ]),
      );
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

  module DocumentContent = {
    let provideTextDocumentContent = (handle: int, resource: Uri.t) =>
      _buildNotification(
        "ExtHostDocumentContentProviders",
        "$provideTextDocumentContent",
        `List([`Int(handle), Uri.to_yojson(resource)]),
      );
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

    let provideDefinition =
        (handle: int, resource: Uri.t, position: OneBasedPosition.t) =>
      _buildNotification(
        "ExtHostLanguageFeatures",
        "$provideDefinition",
        `List([
          `Int(handle),
          Uri.to_yojson(resource),
          OneBasedPosition.to_yojson(position),
        ]),
      );

    let provideDocumentHighlights =
        (handle: int, resource: Uri.t, position: OneBasedPosition.t) =>
      _buildNotification(
        "ExtHostLanguageFeatures",
        "$provideDocumentHighlights",
        `List([
          `Int(handle),
          Uri.to_yojson(resource),
          OneBasedPosition.to_yojson(position),
        ]),
      );
    let provideReferences =
        (handle: int, resource: Uri.t, position: OneBasedPosition.t) =>
      _buildNotification(
        "ExtHostLanguageFeatures",
        "$provideReferences",
        `List([
          `Int(handle),
          Uri.to_yojson(resource),
          OneBasedPosition.to_yojson(position),
          `Assoc([]),
        ]),
      );

    let provideDocumentSymbols = (handle: int, resource: Uri.t) =>
      _buildNotification(
        "ExtHostLanguageFeatures",
        "$provideDocumentSymbols",
        `List([`Int(handle), Uri.to_yojson(resource)]),
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
