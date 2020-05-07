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
  let acceptDocumentsAndEditorsDelta = (~delta, client) => {
    Client.notify(
      ~rpcName="ExtHostDocumentsAndEditors",
      ~method="$acceptDocumentsAndEditorsDelta",
      ~args=`List([DocumentsAndEditorsDelta.to_yojson(delta)]),
      client,
    );
  };
};

module ExtensionService = {
  let activateByEvent = (~event, client) => {
    Client.notify(
      ~rpcName="ExtHostExtensionService",
      ~method="$activateByEvent",
      ~args=`List([`String(event)]),
      client,
    );
  };
};

module LanguageFeatures = {
  let provideCompletionItems =
      (
        ~handle: int,
        ~resource: Uri.t,
        ~position: OneBasedPosition.t,
        ~context: CompletionContext.t,
        client,
      ) => {
    Client.request(
      ~decoder=SuggestResult.decode,
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
        ~decoder=Json.Decode.(list(Location.decode)),
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
      Json.Encode.(encode_value(option(WorkspaceData.encode), workspace));

    Client.notify(
      ~rpcName="ExtHostWorkspace",
      ~method="$initializeWorkspace",
      ~args=`List([json]),
      client,
    );
  };
  let acceptWorkspaceData = (~workspace, client) => {
    let json =
      Json.Encode.(encode_value(option(WorkspaceData.encode), workspace));

    Client.notify(
      ~rpcName="ExtHostWorkspace",
      ~method="$acceptWorkspaceData",
      ~args=`List([json]),
      client,
    );
  };
};
