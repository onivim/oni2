module ExtConfig = Configuration;
open Oni_Core;

module Commands = {
  let executeContributedCommand = (~arguments, ~command, client) => {
    Client.notify(
      ~rpcName="ExtHostCommands",
      ~method="$executeContributedCommand",
      ~args=`List([`String(command), ...arguments]),
      client,
    );
  };
};

module Configuration = {
  open Json.Encode;
  let acceptConfigurationChanged = (~configuration, ~changed, client) => {
    Client.notify(
      ~rpcName="ExtHostConfiguration",
      ~method="$acceptConfigurationChanged",
      ~args=
        `List([
          configuration |> encode_value(ExtConfig.encode),
          changed |> encode_value(ExtConfig.Model.encode),
        ]),
      client,
    );
  };
};

module Decorations = {
  type request = {
    id: int,
    handle: int,
    uri: Uri.t,
  };

  module Encode = {
    let request = request =>
      Json.Encode.(
        obj([
          ("id", request.id |> int),
          ("handle", request.handle |> int),
          ("uri", request.uri |> Uri.encode),
        ])
      );
  };

  type decoration = {
    priority: int,
    bubble: bool,
    title: string,
    letter: string,
    color: ThemeColor.t,
  };

  module Decode = {
    let decoration =
      Json.Decode.(
        Pipeline.(
          decode((priority, bubble, title, letter, color) =>
            {priority, bubble, title, letter, color}
          )
          |> custom(index(0, int))
          |> custom(index(1, bool))
          |> custom(index(2, string))
          |> custom(index(3, string))
          |> custom(index(4, ThemeColor.decode))
        )
      );

    let reply =
      Json.Decode.(
        key_value_pairs(decoration)
        |> map(items =>
             List.fold_left(
               (acc, (id, decoration)) => {
                 let id = int_of_string(id);
                 IntMap.add(id, decoration, acc);
               },
               IntMap.empty,
               items,
             )
           )
      );
  };

  type reply = IntMap.t(decoration);

  let provideDecorations = (~requests, client) => {
    let requestItems =
      requests |> List.map(Json.Encode.encode_value(Encode.request));

    Client.request(
      ~decoder=Decode.reply,
      ~usesCancellationToken=true,
      ~rpcName="ExtHostDecorations",
      ~method="$provideDecorations",
      ~args=`List([`List(requestItems)]),
      client,
    );
  };
};

module DocumentContentProvider = {
  let provideTextDocumentContent = (~handle, ~uri, client) => {
    Client.request(
      ~decoder=Json.Decode.(maybe(string)),
      ~usesCancellationToken=false,
      ~rpcName="ExtHostDocumentContentProviders",
      ~method="$provideTextDocumentContent",
      ~args=`List([`Int(handle), Uri.to_yojson(uri)]),
      client,
    );
  };
};

module Documents = {
  let acceptModelModeChanged = (~uri, ~oldModeId, ~newModeId, client) => {
    Client.notify(
      ~rpcName="ExtHostDocuments",
      ~method="$acceptModelModeChanged",
      ~args=
        `List([
          Uri.to_yojson(uri),
          `String(oldModeId),
          `String(newModeId),
        ]),
      client,
    );
  };

  let acceptModelSaved = (~uri, client) => {
    Client.notify(
      ~rpcName="ExtHostDocuments",
      ~method="$acceptModelSaved",
      ~args=`List([Uri.to_yojson(uri)]),
      client,
    );
  };

  let acceptDirtyStateChanged = (~uri, ~isDirty, client) => {
    Client.notify(
      ~rpcName="ExtHostDocuments",
      ~method="$acceptDirtyStateChanged",
      ~args=`List([Uri.to_yojson(uri), `Bool(isDirty)]),
      client,
    );
  };

  let acceptModelChanged = (~uri, ~modelChangedEvent, ~isDirty, client) => {
    Client.notify(
      ~rpcName="ExtHostDocuments",
      ~method="$acceptModelChanged",
      ~args=
        `List([
          Uri.to_yojson(uri),
          ModelChangedEvent.to_yojson(modelChangedEvent),
          `Bool(isDirty),
        ]),
      client,
    );
  };
};

module DocumentsAndEditors = {
  open Json.Encode;
  let acceptDocumentsAndEditorsDelta = (~delta, client) => {
    Client.notify(
      ~rpcName="ExtHostDocumentsAndEditors",
      ~method="$acceptDocumentsAndEditorsDelta",
      ~args=`List([delta |> encode_value(DocumentsAndEditorsDelta.encode)]),
      client,
    );
  };
};

module ExtensionService = {
  open Json.Encode;
  let activateByEvent = (~event, client) => {
    Client.notify(
      ~rpcName="ExtHostExtensionService",
      ~method="$activateByEvent",
      ~args=`List([`String(event)]),
      client,
    );
  };

  let activate = (~extensionId, ~reason, client) => {
    Client.request(
      ~decoder=Json.Decode.bool,
      ~rpcName="ExtHostExtensionService",
      ~method="$activate",
      ~args=
        `List([
          extensionId |> encode_value(string),
          reason |> encode_value(ExtensionActivationReason.encode),
        ]),
      client,
    );
  };

  let deltaExtensions = (~toAdd, ~toRemove, client) => {
    Client.request(
      ~decoder=Json.Decode.null,
      ~rpcName="ExtHostExtensionService",
      ~method="$deltaExtensions",
      ~args=
        `List([
          `List(
            toAdd |> List.map(Exthost_Extension.InitData.Extension.to_yojson),
          ),
          `List(toRemove |> List.map(encode_value(ExtensionId.encode))),
        ]),
      client,
    );
  };
};

module LanguageFeatures = {
  let provideCodeLenses = (~handle: int, ~resource: Uri.t, client) => {
    let decoder = Json.Decode.(nullable(CodeLens.List.decode));

    Client.request(
      ~decoder,
      ~usesCancellationToken=true,
      ~rpcName="ExtHostLanguageFeatures",
      ~method="$provideCodeLenses",
      ~args=`List([`Int(handle), Uri.to_yojson(resource)]),
      client,
    );
  };
  let provideCompletionItems =
      (
        ~handle: int,
        ~resource: Uri.t,
        ~position: OneBasedPosition.t,
        ~context: CompletionContext.t,
        client,
      ) => {
    // It's possible to get a null result from completion providers,
    // so we need to handle that here - we just treat it as an
    // empty set of suggestions.
    let decoder =
      Json.Decode.(
        nullable(SuggestResult.decode)
        |> map(
             fun
             | Some(suggestResult) => suggestResult
             | None => SuggestResult.empty,
           )
      );

    Client.request(
      ~decoder,
      ~usesCancellationToken=true,
      ~rpcName="ExtHostLanguageFeatures",
      ~method="$provideCompletionItems",
      ~args=
        `List([
          `Int(handle),
          Uri.to_yojson(resource),
          OneBasedPosition.to_yojson(position),
          CompletionContext.to_yojson(context),
        ]),
      client,
    );
  };

  module Internal = {
    let provideDefinitionLink =
        (~handle, ~resource, ~position, method, client) => {
      Client.request(
        ~decoder=Json.Decode.(list(DefinitionLink.decode)),
        ~usesCancellationToken=true,
        ~rpcName="ExtHostLanguageFeatures",
        ~method,
        ~args=
          `List([
            `Int(handle),
            Uri.to_yojson(resource),
            OneBasedPosition.to_yojson(position),
          ]),
        client,
      );
    };
  };
  let provideDocumentHighlights = (~handle, ~resource, ~position, client) => {
    Client.request(
      ~decoder=Json.Decode.(list(DocumentHighlight.decode)),
      ~usesCancellationToken=true,
      ~rpcName="ExtHostLanguageFeatures",
      ~method="$provideDocumentHighlights",
      ~args=
        `List([
          `Int(handle),
          Uri.to_yojson(resource),
          OneBasedPosition.to_yojson(position),
        ]),
      client,
    );
  };

  let provideDocumentSymbols = (~handle, ~resource, client) => {
    Client.request(
      ~decoder=Json.Decode.(list(DocumentSymbol.decode)),
      ~usesCancellationToken=true,
      ~rpcName="ExtHostLanguageFeatures",
      ~method="$provideDocumentSymbols",
      ~args=`List([`Int(handle), Uri.to_yojson(resource)]),
      client,
    );
  };

  let provideDefinition = (~handle, ~resource, ~position, client) =>
    Internal.provideDefinitionLink(
      ~handle,
      ~resource,
      ~position,
      "$provideDefinition",
      client,
    );
  let provideDeclaration = (~handle, ~resource, ~position, client) =>
    Internal.provideDefinitionLink(
      ~handle,
      ~resource,
      ~position,
      "$provideDeclaration",
      client,
    );

  let provideHover = (~handle, ~resource, ~position, client) => {
    Client.request(
      ~decoder=Json.Decode.(nullable(Hover.decode)),
      ~usesCancellationToken=true,
      ~rpcName="ExtHostLanguageFeatures",
      ~method="$provideHover",
      ~args=
        `List([
          `Int(handle),
          Uri.to_yojson(resource),
          OneBasedPosition.to_yojson(position),
        ]),
      client,
    );
  };
  let provideImplementation = (~handle, ~resource, ~position, client) =>
    Internal.provideDefinitionLink(
      ~handle,
      ~resource,
      ~position,
      "$provideImplementation",
      client,
    );
  let provideTypeDefinition = (~handle, ~resource, ~position, client) =>
    Internal.provideDefinitionLink(
      ~handle,
      ~resource,
      ~position,
      "$provideTypeDefinition",
      client,
    );

  let provideReferences = (~handle, ~resource, ~position, ~context, client) => {
    Client.request(
      ~decoder=Json.Decode.(list(Location.decode)),
      ~usesCancellationToken=true,
      ~rpcName="ExtHostLanguageFeatures",
      ~method="$provideReferences",
      ~args=
        `List([
          `Int(handle),
          Uri.to_yojson(resource),
          OneBasedPosition.to_yojson(position),
          context |> Json.Encode.encode_value(ReferenceContext.encode),
        ]),
      client,
    );
  };

  let provideSignatureHelp = (~handle, ~resource, ~position, ~context, client) => {
    Client.request(
      ~decoder=Json.Decode.(nullable(SignatureHelp.Response.decode)),
      ~usesCancellationToken=true,
      ~rpcName="ExtHostLanguageFeatures",
      ~method="$provideSignatureHelp",
      ~args=
        `List([
          `Int(handle),
          Uri.to_yojson(resource),
          OneBasedPosition.to_yojson(position),
          context
          |> Json.Encode.encode_value(SignatureHelp.RequestContext.encode),
        ]),
      client,
    );
  };

  let releaseSignatureHelp = (~handle, ~id, client) =>
    Client.notify(
      ~rpcName="ExtHostLanguageFeatures",
      ~method="$releaseSignatureHelp",
      ~args=
        `List(
          Json.Encode.[
            handle |> encode_value(int),
            id |> encode_value(int),
          ],
        ),
      client,
    );

  let provideDocumentFormattingEdits = (~handle, ~resource, ~options, client) => {
    Client.request(
      ~decoder=Json.Decode.(nullable(list(Edit.SingleEditOperation.decode))),
      ~usesCancellationToken=true,
      ~rpcName="ExtHostLanguageFeatures",
      ~method="$provideDocumentFormattingEdits",
      ~args=
        `List([
          `Int(handle),
          Uri.to_yojson(resource),
          options |> Json.Encode.encode_value(FormattingOptions.encode),
        ]),
      client,
    );
  };

  let provideDocumentRangeFormattingEdits =
      (~handle, ~resource, ~range, ~options, client) => {
    Client.request(
      ~decoder=Json.Decode.(nullable(list(Edit.SingleEditOperation.decode))),
      ~usesCancellationToken=true,
      ~rpcName="ExtHostLanguageFeatures",
      ~method="$provideDocumentRangeFormattingEdits",
      ~args=
        `List([
          `Int(handle),
          Uri.to_yojson(resource),
          OneBasedRange.to_yojson(range),
          options |> Json.Encode.encode_value(FormattingOptions.encode),
        ]),
      client,
    );
  };

  let provideOnTypeFormattingEdits =
      (~handle, ~resource, ~position, ~character, ~options, client) => {
    Client.request(
      ~decoder=Json.Decode.(nullable(list(Edit.SingleEditOperation.decode))),
      ~usesCancellationToken=true,
      ~rpcName="ExtHostLanguageFeatures",
      ~method="$provideOnTypeFormattingEdits",
      ~args=
        `List([
          `Int(handle),
          Uri.to_yojson(resource),
          OneBasedPosition.to_yojson(position),
          `String(character),
          options |> Json.Encode.encode_value(FormattingOptions.encode),
        ]),
      client,
    );
  };

  let provideRenameEdits = (~handle, ~resource, ~position, ~newName: string) => {
    Client.request(
      ~decoder=Json.Decode.(nullable(WorkspaceEdit.decode)),
      ~usesCancellationToken=true,
      ~rpcName="ExtHostLanguageFeatures",
      ~method="$provideRenameEdits",
      ~args=
        `List([
          `Int(handle),
          Uri.to_yojson(resource),
          OneBasedPosition.to_yojson(position),
          `String(newName),
        ]),
    );
  };

  let resolveRenameLocation = (~handle, ~resource, ~position) => {
    Client.request(
      ~decoder=Json.Decode.(nullable(RenameLocation.decode)),
      ~usesCancellationToken=true,
      ~rpcName="ExtHostLanguageFeatures",
      ~method="$resolveRenameLocation",
      ~args=
        `List([
          `Int(handle),
          Uri.to_yojson(resource),
          OneBasedPosition.to_yojson(position),
        ]),
    );
  };
};

module SCM = {
  let provideOriginalResource = (~handle, ~uri, client) => {
    Client.request(
      ~decoder=Json.Decode.(maybe(Uri.decode)),
      ~usesCancellationToken=true,
      ~rpcName="ExtHostSCM",
      ~method="$provideOriginalResource",
      ~args=`List([`Int(handle), Uri.to_yojson(uri)]),
      client,
    );
  };

  let onInputBoxValueChange = (~handle, ~value, client) => {
    Client.notify(
      ~usesCancellationToken=false,
      ~rpcName="ExtHostSCM",
      ~method="$onInputBoxValueChange",
      ~args=`List([`Int(handle), `String(value)]),
      client,
    );
  };
};

module TerminalService = {
  let spawnExtHostProcess =
      (
        ~id,
        ~shellLaunchConfig,
        ~activeWorkspaceRoot,
        ~cols,
        ~rows,
        ~isWorkspaceShellAllowed,
        client,
      ) => {
    Client.notify(
      ~rpcName="ExtHostTerminalService",
      ~method="$spawnExtHostProcess",
      ~args=
        `List([
          `Int(id),
          ShellLaunchConfig.to_yojson(shellLaunchConfig),
          Uri.to_yojson(activeWorkspaceRoot),
          `Int(cols),
          `Int(rows),
          `Bool(isWorkspaceShellAllowed),
        ]),
      client,
    );
  };

  let acceptProcessInput = (~id, ~data, client) => {
    Client.notify(
      ~rpcName="ExtHostTerminalService",
      ~method="$acceptProcessInput",
      ~args=`List([`Int(id), `String(data)]),
      client,
    );
  };

  let acceptProcessResize = (~id, ~cols, ~rows, client) => {
    Client.notify(
      ~rpcName="ExtHostTerminalService",
      ~method="$acceptProcessResize",
      ~args=`List([`Int(id), `Int(cols), `Int(rows)]),
      client,
    );
  };

  let acceptProcessShutdown = (~id, ~immediate, client) => {
    Client.notify(
      ~rpcName="ExtHostTerminalService",
      ~method="$acceptProcessShutdown",
      ~args=`List([`Int(id), `Bool(immediate)]),
      client,
    );
  };
};

module Workspace = {
  let initializeWorkspace = (~workspace, client) => {
    let json =
      Json.Encode.(encode_value(nullable(WorkspaceData.encode), workspace));

    Client.notify(
      ~rpcName="ExtHostWorkspace",
      ~method="$initializeWorkspace",
      ~args=`List([json]),
      client,
    );
  };
  let acceptWorkspaceData = (~workspace, client) => {
    let json =
      Json.Encode.(encode_value(nullable(WorkspaceData.encode), workspace));

    Client.notify(
      ~rpcName="ExtHostWorkspace",
      ~method="$acceptWorkspaceData",
      ~args=`List([json]),
      client,
    );
  };
};
