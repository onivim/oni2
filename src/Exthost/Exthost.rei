open EditorCoreTypes;
open Oni_Core;

module Extension = Exthost_Extension;
module Protocol = Exthost_Protocol;
module Transport = Exthost_Transport;

module CompletionContext: {
  type triggerKind =
    | Invoke
    | TriggerCharacter
    | TriggerForIncompleteCompletions;

  type t = {
    triggerKind,
    triggerCharacter: option(string),
  };
};

module CompletionKind: {
  type t =
    | Method
    | Function
    | Constructor
    | Field
    | Variable
    | Class
    | Struct
    | Interface
    | Module
    | Property
    | Event
    | Operator
    | Unit
    | Value
    | Constant
    | Enum
    | EnumMember
    | Keyword
    | Text
    | Color
    | File
    | Reference
    | Customcolor
    | Folder
    | TypeParameter
    | User
    | Issue
    | Snippet;

  let ofInt: int => option(t);
};

module Location: {
  type t = {
    uri: Uri.t,
    range: OneBasedRange.t,
  };

  let decode: Json.decoder(t);
};

module DocumentFilter: {
  [@deriving show]
  type t = {
    language: option(string),
    scheme: option(string),
    exclusive: bool,
  };

  let decode: Json.decoder(t);
};

module DocumentHighlight: {
  module Kind: {
    [@deriving show]
    type t =
      | Text
      | Read
      | Write;

    let ofInt: int => option(t);
    let toInt: t => int;
    let decode: Json.decoder(t);
  };

  [@deriving show]
  type t = {
    range: OneBasedRange.t,
    kind: Kind.t,
  };

  let decode: Json.decoder(t);
};

module SuggestItem: {
  type t = {
    label: string,
    kind: CompletionKind.t,
    detail: option(string),
    documentation: option(string),
    sortText: option(string),
    filterText: option(string),
    insertText: option(string),
  };

  let decode: Json.decoder(t);
};

module ReferenceContext: {
  type t = {includeDeclaration: bool};

  let encode: Json.encoder(t);
};

module SuggestResult: {
  type t = {
    completions: list(SuggestItem.t),
    isIncomplete: bool,
  };

  let decode: Json.decoder(t);
};

module SymbolKind: {
  [@deriving show]
  type t =
    | File
    | Module
    | Namespace
    | Package
    | Class
    | Method
    | Property
    | Field
    | Constructor
    | Enum
    | Interface
    | Function
    | Variable
    | Constant
    | String
    | Number
    | Boolean
    | Array
    | Object
    | Key
    | Null
    | EnumMember
    | Struct
    | Event
    | Operator
    | TypeParameter;

  let toInt: t => int;
  let ofInt: int => option(t);
  let decode: Json.decoder(t);
};

module DocumentSymbol: {
  [@deriving show]
  type t = {
    name: string,
    detail: string,
    kind: SymbolKind.t,
    // TODO: tags
    containerName: option(string),
    range: OneBasedRange.t,
    selectionRange: OneBasedRange.t,
    children: list(t),
  };

  let decode: Json.decoder(t);
};

module Configuration: {
  // Type relating to 'ConfigurationModel' in VSCode
  // This is an 'instance' of configuration - modelling user, workspace, or default configuration.
  // The full configuration is set up by combining the various configuration 'instances'.
  module Model: {
    type t;

    let empty: t;
    let create: (~keys: list(string), Json.t) => t;
    let to_yojson: t => Json.t;
    let encode: Json.encoder(t);
    let fromSettings: Config.Settings.t => t;
    let toString: t => string;
  };

  type t;

  let to_yojson: t => Json.t;
  let encode: Json.encoder(t);
  let empty: t;
  let create:
    (~defaults: Model.t=?, ~user: Model.t=?, ~workspace: Model.t=?, unit) => t;
};

module Eol: {
  type t =
    | LF
    | CRLF;

  let default: t;

  let toString: t => string;

  let to_yojson: t => Yojson.Safe.t;
};

module ModelAddedDelta: {
  type t = {
    uri: Uri.t,
    versionId: int,
    lines: list(string),
    eol: Eol.t,
    modeId: string,
    isDirty: bool,
  };

  let create:
    (
      ~versionId: int=?,
      ~lines: list(string)=?,
      ~eol: Eol.t=?,
      ~isDirty: bool=?,
      ~modeId: string,
      Uri.t
    ) =>
    t;

  let to_yojson: t => Yojson.Safe.t;
};

module DocumentsAndEditorsDelta: {
  type t = {
    removedDocuments: list(Uri.t),
    addedDocuments: list(ModelAddedDelta.t),
    removedEditors: list(string),
    addedEditors: list(string),
  };

  let create:
    (
      ~removedDocuments: list(Uri.t),
      ~addedDocuments: list(ModelAddedDelta.t)
    ) =>
    t;

  let to_yojson: t => Yojson.Safe.t;
};

module OneBasedPosition: {
  type t = {
    lineNumber: int,
    column: int,
  };

  let ofPosition: EditorCoreTypes.Location.t => t;
  let to_yojson: t => Yojson.Safe.t;
};

module ModelContentChange: {
  type t = {
    range: OneBasedRange.t,
    text: string,
  };

  let ofBufferUpdate: (BufferUpdate.t, Eol.t) => t;

  let to_yojson: t => Yojson.Safe.t;
};

module ModelChangedEvent: {
  type t = {
    changes: list(ModelContentChange.t),
    eol: Eol.t,
    versionId: int,
  };

  let to_yojson: t => Yojson.Safe.t;
};

module OneBasedRange: {
  [@deriving show]
  type t = {
    startLineNumber: int,
    endLineNumber: int,
    startColumn: int,
    endColumn: int,
  };

  let ofRange: Range.t => t;
  let toRange: t => Range.t;

  let decode: Json.decoder(t);
};

module ShellLaunchConfig: {
  type t = {
    name: string,
    executable: string,
    arguments: list(string),
  };

  let to_yojson: t => Yojson.Safe.t;
};

module WorkspaceData: {
  module Folder: {
    type t = {
      uri: Uri.t,
      name: string,
      index: int,
    };

    let encode: Json.encoder(t);
    let decode: Json.decoder(t);
  };

  type t = {
    folders: list(Folder.t),
    id: string,
    name: string,
    configuration: option(Uri.t),
    isUntitled: bool,
  };

  let encode: Json.encoder(t);
  let decode: Json.decoder(t);
};

module Msg: {
  module Commands: {
    [@deriving show]
    type msg =
      | RegisterCommand(string)
      | UnregisterCommand(string)
      | ExecuteCommand({
          command: string,
          args: list(Yojson.Safe.t),
          retry: bool,
        })
      | GetCommands;
  };

  module DebugService: {
    [@deriving show]
    type msg =
      | RegisterDebugTypes(list(string));
  };

  module Decorations: {
    [@deriving show]
    type msg =
      | RegisterDecorationProvider({
          handle: int,
          label: string,
        })
      | UnregisterDecorationProvider({handle: int})
      | DecorationsDidChange({
          handle: int,
          uris: list(Oni_Core.Uri.t),
        });
  };

  module Diagnostics: {
    [@deriving show]
    type entry = (Uri.t, [@opaque] list(Diagnostic.t));
    [@deriving show]
    type msg =
      | ChangeMany({
          owner: string,
          entries: list(entry),
        })
      | Clear({owner: string});
  };

  module DocumentContentProvider: {
    [@deriving show]
    type msg =
      | RegisterTextContentProvider({
          handle: int,
          scheme: string,
        })
      | UnregisterTextContentProvider({handle: int})
      | VirtualDocumentChange({
          uri: Oni_Core.Uri.t,
          value: string,
        });
  };

  module ExtensionService: {
    [@deriving show]
    type msg =
      | ActivateExtension({
          extensionId: string,
          activationEvent: option(string),
        })
      | WillActivateExtension({extensionId: string})
      | DidActivateExtension({
          extensionId: string,
          //startup: bool,
          codeLoadingTime: int,
          activateCallTime: int,
          activateResolvedTime: int,
        })
      //activationEvent: option(string),
      | ExtensionActivationError({
          extensionId: string,
          errorMessage: string,
        })
      | ExtensionRuntimeError({extensionId: string});
  };

  module LanguageFeatures: {
    [@deriving show]
    type msg =
      | RegisterDocumentHighlightProvider({
          handle: int,
          selector: list(DocumentFilter.t),
        })
      | RegisterDocumentSymbolProvider({
          handle: int,
          selector: list(DocumentFilter.t),
          label: string,
        })
      | RegisterDefinitionSupport({
          handle: int,
          selector: list(DocumentFilter.t),
        })
      | RegisterDeclarationSupport({
          handle: int,
          selector: list(DocumentFilter.t),
        })
      | RegisterImplementationSupport({
          handle: int,
          selector: list(DocumentFilter.t),
        })
      | RegisterTypeDefinitionSupport({
          handle: int,
          selector: list(DocumentFilter.t),
        })
      | RegisterSuggestSupport({
          handle: int,
          selector: list(DocumentFilter.t),
          triggerCharacters: list(string),
          supportsResolveDetails: bool,
          extensionId: string,
        })
      | RegisterReferenceSupport({
          handle: int,
          selector: list(DocumentFilter.t),
        })
      | Unregister({handle: int});
  };

  module MessageService: {
    type severity =
      | Ignore
      | Info
      | Warning
      | Error;

    [@deriving show]
    type msg =
      | ShowMessage({
          severity,
          message: string,
          extensionId: option(string),
        });
  };

  module Telemetry: {
    [@deriving show]
    type msg =
      | PublicLog({
          eventName: string,
          data: Yojson.Safe.t,
        })
      | PublicLog2({
          eventName: string,
          data: Yojson.Safe.t,
        });
  };

  module TerminalService: {
    [@deriving show]
    type msg =
      | SendProcessTitle({
          terminalId: int,
          title: string,
        })
      | SendProcessData({
          terminalId: int,
          data: string,
        })
      | SendProcessPid({
          terminalId: int,
          pid: int,
        })
      | SendProcessExit({
          terminalId: int,
          exitCode: int,
        });
  };

  module StatusBar: {
    [@deriving show]
    type alignment =
      | Left
      | Right;

    [@deriving show]
    type msg =
      | SetEntry({
          id: string,
          text: string,
          source: string,
          alignment,
          priority: int,
        });
  };

  [@deriving show]
  type t =
    | Connected
    | Ready
    | Commands(Commands.msg)
    | DebugService(DebugService.msg)
    | Decorations(Decorations.msg)
    | Diagnostics(Diagnostics.msg)
    | DocumentContentProvider(DocumentContentProvider.msg)
    | ExtensionService(ExtensionService.msg)
    | LanguageFeatures(LanguageFeatures.msg)
    | MessageService(MessageService.msg)
    | StatusBar(StatusBar.msg)
    | Telemetry(Telemetry.msg)
    | TerminalService(TerminalService.msg)
    | Initialized
    | Disconnected
    | Unhandled
    | Unknown({
        method: string,
        args: Yojson.Safe.t,
      });
};

module NamedPipe: {
  type t;

  let create: string => t;
  let toString: t => string;
};

module Client: {
  type t;

  // TODO
  type reply = unit;

  let start:
    (
      ~initialConfiguration: Configuration.t=?,
      ~namedPipe: NamedPipe.t,
      ~initData: Extension.InitData.t,
      ~handler: Msg.t => option(reply),
      ~onError: string => unit,
      unit
    ) =>
    result(t, string);

  let close: t => unit;

  let terminate: t => unit;

  module Testing: {let getPendingRequestCount: t => int;};
};

module Request: {
  module Commands: {
    let executeContributedCommand:
      (~arguments: list(Json.t), ~command: string, Client.t) => unit;
  };

  module Configuration: {
    let acceptConfigurationChanged:
      (
        ~configuration: Configuration.t,
        ~changed: Configuration.Model.t,
        Client.t
      ) =>
      unit;
  };

  module DocumentContentProvider: {
    let provideTextDocumentContent:
      (~handle: int, ~uri: Uri.t, Client.t) => Lwt.t(option(string));
  };

  module Documents: {
    let acceptModelModeChanged:
      (~uri: Uri.t, ~oldModeId: string, ~newModeId: string, Client.t) => unit;

    let acceptModelSaved: (~uri: Uri.t, Client.t) => unit;

    let acceptDirtyStateChanged:
      (~uri: Uri.t, ~isDirty: bool, Client.t) => unit;

    let acceptModelChanged:
      (
        ~uri: Uri.t,
        ~modelChangedEvent: ModelChangedEvent.t,
        ~isDirty: bool,
        Client.t
      ) =>
      unit;
  };

  module DocumentsAndEditors: {
    let acceptDocumentsAndEditorsDelta:
      (~delta: DocumentsAndEditorsDelta.t, Client.t) => unit;
  };

  module ExtensionService: {
    let activateByEvent: (~event: string, Client.t) => unit;
  };

  module LanguageFeatures: {
    let provideCompletionItems:
      (
        ~handle: int,
        ~resource: Uri.t,
        ~position: OneBasedPosition.t,
        ~context: CompletionContext.t,
        Client.t
      ) =>
      Lwt.t(SuggestResult.t);

    let provideDocumentHighlights:
      (
        ~handle: int,
        ~resource: Uri.t,
        ~position: OneBasedPosition.t,
        Client.t
      ) =>
      Lwt.t(list(DocumentHighlight.t));

    let provideDocumentSymbols:
      (~handle: int, ~resource: Uri.t, Client.t) =>
      Lwt.t(list(DocumentSymbol.t));

    let provideDefinition:
      (
        ~handle: int,
        ~resource: Uri.t,
        ~position: OneBasedPosition.t,
        Client.t
      ) =>
      Lwt.t(list(Location.t));

    let provideDeclaration:
      (
        ~handle: int,
        ~resource: Uri.t,
        ~position: OneBasedPosition.t,
        Client.t
      ) =>
      Lwt.t(list(Location.t));

    let provideImplementation:
      (
        ~handle: int,
        ~resource: Uri.t,
        ~position: OneBasedPosition.t,
        Client.t
      ) =>
      Lwt.t(list(Location.t));

    let provideReferences:
      (
        ~handle: int,
        ~resource: Uri.t,
        ~position: OneBasedPosition.t,
        ~context: ReferenceContext.t,
        Client.t
      ) =>
      Lwt.t(list(Location.t));

    let provideTypeDefinition:
      (
        ~handle: int,
        ~resource: Uri.t,
        ~position: OneBasedPosition.t,
        Client.t
      ) =>
      Lwt.t(list(Location.t));
  };

  module TerminalService: {
    let spawnExtHostProcess:
      (
        ~id: int,
        ~shellLaunchConfig: ShellLaunchConfig.t,
        ~activeWorkspaceRoot: Uri.t,
        ~cols: int,
        ~rows: int,
        ~isWorkspaceShellAllowed: bool,
        Client.t
      ) =>
      unit;

    let acceptProcessInput: (~id: int, ~data: string, Client.t) => unit;
    let acceptProcessResize:
      (~id: int, ~cols: int, ~rows: int, Client.t) => unit;
    let acceptProcessShutdown: (~id: int, ~immediate: bool, Client.t) => unit;
  };

  module Workspace: {
    let initializeWorkspace:
      (~workspace: option(WorkspaceData.t), Client.t) => unit;
    let acceptWorkspaceData:
      (~workspace: option(WorkspaceData.t), Client.t) => unit;
  };
};
