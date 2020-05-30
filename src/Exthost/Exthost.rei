open EditorCoreTypes;
open Oni_Core;

module Extension = Exthost_Extension;
module Protocol = Exthost_Protocol;
module Transport = Exthost_Transport;

module Command: {
  [@deriving show]
  type t = {
    id: string,
    title: option(string),
  };

  let decode: Json.decoder(t);
};

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

module OneBasedRange: {
  type t = {
    startLineNumber: int,
    endLineNumber: int,
    startColumn: int,
    endColumn: int,
  };

  let ofRange: Range.t => t;
  let toRange: t => Range.t;
};

module Location: {
  type t = {
    uri: Uri.t,
    range: OneBasedRange.t,
  };

  let decode: Json.decoder(t);
};

module MarkdownString: {
  type t = string;

  let decode: Json.decoder(t);
};

module Edit: {
  module SingleEditOperation: {
    type t = {
      range: OneBasedRange.t,
      text: option(string),
      forceMoveMarkers: bool,
    };

    let decode: Json.decoder(t);
  };
};

module ExtensionId: {
  [@deriving show]
  type t = string;

  let decode: Json.decoder(t);
};

module DefinitionLink: {
  type t = {
    uri: Uri.t,
    range: OneBasedRange.t,
    originSelectionRange: option(OneBasedRange.t),
    targetSelectionRange: option(OneBasedRange.t),
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

  let matches: (~filetype: string, t) => bool;

  let decode: Json.decoder(t);
};

module DocumentSelector: {
  [@deriving show]
  type t = list(DocumentFilter.t);

  let matches: (~filetype: string, t) => bool;

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

module Hover: {
  type t = {
    contents: list(MarkdownString.t),
    range: option(OneBasedRange.t),
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

module Label: {
  [@deriving show]
  type segment =
    | Text(string)
    | Icon(string);

  [@deriving show]
  type t = list(segment);

  let of_string: string => t;

  let decode: Json.decoder(t);
};

module SCM: {
  [@deriving show({with_path: false})]
  type command = {
    id: string,
    title: string,
    tooltip: option(string),
    arguments: list([@opaque] Json.t),
  };

  module Resource: {
    module Icons: {
      [@deriving show({with_path: false})]
      type t = {
        light: Uri.t,
        dark: Uri.t,
      };
    };

    [@deriving show({with_path: false})]
    type t = {
      handle: int,
      uri: Uri.t,
      //      icons: Icons.t,
      tooltip: string,
      strikeThrough: bool,
      faded: bool,
    };

    let decode: Json.decoder(t);

    module Splice: {
      type nonrec t = {
        start: int,
        deleteCount: int,
        resources: list(t),
      };
    };

    module Splices: {
      [@deriving show({with_path: false})]
      type t = {
        handle: int,
        resourceSplices: [@opaque] list(Splice.t),
      };
    };

    module Decode: {let splices: Json.decoder(Splices.t);};
  };

  module Decode: {
    //let resource: Yojson.Safe.t => Resource.t;
    let command: Yojson.Safe.t => option(command);
  };
};

module SignatureHelp: {
  module ProviderMetadata: {
    [@deriving show]
    type t = {
      triggerCharacters: list(string),
      retriggerCharacters: list(string),
    };

    let decode: Json.decoder(t);
  };

  module TriggerKind: {
    [@deriving show]
    type t =
      | Invoke
      | TriggerCharacter
      | ContentChange;
  };

  module RequestContext: {
    [@deriving show]
    type t = {
      triggerKind: TriggerKind.t,
      triggerCharacter: option(string),
      isRetrigger: bool,
      // TODO: Active signature help?
      //activate
    };

    let encode: Json.encoder(t);
  };

  module ParameterInformation: {
    [@deriving show]
    type t = {
      label: string,
      // TODO
      //documentation: option(string),
    };
    let decode: Json.decoder(t);
  };

  module Signature: {
    [@deriving show]
    type t = {
      label: string,
      // TODO:
      //documentation: options(MarkdownString.t),
      parameters: list(ParameterInformation.t),
    };

    let decode: Json.decoder(t);
  };

  module Response: {
    type t = {
      id: int,
      signatures: list(Signature.t),
      activeSignature: int,
      activeParameter: int,
    };

    let decode: Json.decoder(t);
  };
};

module SuggestResult: {
  type t = {
    completions: list(SuggestItem.t),
    isIncomplete: bool,
  };

  let empty: t;

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

module Diagnostic: {
  type t = {
    range: OneBasedRange.t,
    message: string,
    severity: int,
  };

  let decode: Json.decoder(t);
};

module Eol: {
  type t =
    | LF
    | CRLF;

  let default: t;

  let toString: t => string;

  let encode: Json.encoder(t);
};

module FormattingOptions: {
  type t = {
    tabSize: int,
    insertSpaces: bool,
  };

  let encode: Json.encoder(t);
};

module Files: {
  module FileSystemProviderCapabilities: {
    type capability = [
      | `FileReadWrite
      | `FileOpenReadWriteClose
      | `FileReadStream
      | `FileFolderCopy
      | `PathCaseSensitive
      | `Readonly
      | `Trash
    ];

    [@deriving show]
    type t;

    let test: (capability, t) => bool;

    let decode: Json.decoder(t);
  };

  module FileChangeType: {
    [@deriving show]
    type t =
      | Updated
      | Added
      | Deleted;

    let ofInt: int => option(t);
    let toInt: t => int;

    let decode: Json.decoder(t);
  };

  module FileChange: {
    [@deriving show]
    type t = {
      resource: Uri.t,
      changeType: FileChangeType.t,
    };

    let decode: Json.decoder(t);
  };

  module FileType: {
    [@deriving show]
    type t =
      | Unknown
      | File
      | Directory
      | SymbolicLink;

    let ofInt: int => option(t);
    let toInt: t => int;

    let decode: Json.decoder(t);
    let encode: Json.encoder(t);
  };

  module FileOverwriteOptions: {
    [@deriving show]
    type t = {overwrite: bool};

    let decode: Json.decoder(t);
  };

  module FileWriteOptions: {
    [@deriving show]
    type t = {
      overwrite: bool,
      create: bool,
    };

    let decode: Json.decoder(t);
  };
  module FileOpenOptions: {
    [@deriving show]
    type t = {create: bool};

    let decode: Json.decoder(t);
  };
  module FileDeleteOptions: {
    [@deriving show]
    type t = {
      recursive: bool,
      useTrash: bool,
    };

    let decode: Json.decoder(t);
  };

  module StatResult: {
    [@deriving show]
    type t = {
      fileType: FileType.t,
      mtime: int,
      ctime: int,
      size: int,
    };

    let decode: Json.decoder(t);
    let encode: Json.encoder(t);
  };
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

  let encode: Json.encoder(t);
};

module TextEditor: {
  module CursorStyle: {
    type t =
      | Hidden // 0
      | Blink // 1
      | Smooth // 2
      | Phase // 3
      | Expand // 4
      | Solid; // 5;

    let encode: Json.encoder(t);
  };

  module LineNumbersStyle: {
    type t =
      | Off // 0
      | On // 1
      | Relative; // 2

    let encode: Json.encoder(t);
  };

  module ResolvedConfiguration: {
    type t = {
      tabSize: int,
      indentSize: int,
      insertSpaces: int,
      cursorStyle: CursorStyle.t,
      lineNumbers: LineNumbersStyle.t,
    };

    let encode: Json.encoder(t);
  };

  module AddData: {
    type t = {
      id: string,
      documentUri: Uri.t,
      options: ResolvedConfiguration.t,
      // TODO:
      // selections: list(Selection.t),
      // visibleRanges: list(Range.t),
      // editorPosition: option(EditorViewColumn.t),
    };

    let encode: Json.encoder(t);
  };
};

module DocumentsAndEditorsDelta: {
  type t = {
    removedDocuments: list(Uri.t),
    addedDocuments: list(ModelAddedDelta.t),
    removedEditors: list(string),
    addedEditors: list(TextEditor.AddData.t),
    newActiveEditor: option(string),
  };

  let create:
    (
      ~removedDocuments: list(Uri.t)=?,
      ~addedDocuments: list(ModelAddedDelta.t)=?,
      ~removedEditors: list(string)=?,
      ~addedEditors: list(TextEditor.AddData.t)=?,
      ~newActiveEditor: option(string)=?,
      unit
    ) =>
    t;

  let encode: Json.encoder(t);
};

module OneBasedPosition: {
  [@deriving show({with_path: false})]
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

  let fromUri: (~name: string, ~id: string, Uri.t) => t;
  let fromPath: string => t;

  let encode: Json.encoder(t);
  let decode: Json.decoder(t);
};

module ThemeColor: {
  type t = {id: string};

  let decode: Json.decoder(t);
};

module Msg: {
  module Clipboard: {
    [@deriving show]
    type msg =
      | ReadText
      | WriteText(string);
  };

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

  module FileSystem: {
    open Files;

    [@deriving show]
    type msg =
      | RegisterFileSystemProvider({
          handle: int,
          scheme: string,
          capabilities: FileSystemProviderCapabilities.t,
        })
      | UnregisterProvider({handle: int})
      | OnFileSystemChange({
          handle: int,
          resource: list(FileChange.t),
        })
      | Stat({uri: Uri.t})
      | ReadDir({uri: Uri.t})
      | ReadFile({uri: Uri.t})
      | WriteFile({
          uri: Uri.t,
          bytes: Bytes.t,
        })
      | Rename({
          source: Uri.t,
          target: Uri.t,
          opts: FileOverwriteOptions.t,
        })
      | Copy({
          source: Uri.t,
          target: Uri.t,
          opts: FileOverwriteOptions.t,
        })
      | Mkdir({uri: Uri.t})
      | Delete({
          uri: Uri.t,
          opts: FileDeleteOptions.t,
        });
  };

  module LanguageFeatures: {
    [@deriving show]
    type msg =
      | RegisterDocumentHighlightProvider({
          handle: int,
          selector: DocumentSelector.t,
        })
      | RegisterDocumentSymbolProvider({
          handle: int,
          selector: DocumentSelector.t,
          label: string,
        })
      | RegisterDefinitionSupport({
          handle: int,
          selector: DocumentSelector.t,
        })
      | RegisterDeclarationSupport({
          handle: int,
          selector: DocumentSelector.t,
        })
      | RegisterHoverProvider({
          handle: int,
          selector: DocumentSelector.t,
        })
      | RegisterImplementationSupport({
          handle: int,
          selector: DocumentSelector.t,
        })
      | RegisterTypeDefinitionSupport({
          handle: int,
          selector: DocumentSelector.t,
        })
      | RegisterSignatureHelpProvider({
          handle: int,
          selector: DocumentSelector.t,
          metadata: SignatureHelp.ProviderMetadata.t,
        })
      | RegisterSuggestSupport({
          handle: int,
          selector: DocumentSelector.t,
          triggerCharacters: list(string),
          supportsResolveDetails: bool,
          extensionId: string,
        })
      | RegisterReferenceSupport({
          handle: int,
          selector: DocumentSelector.t,
        })
      | RegisterDocumentFormattingSupport({
          handle: int,
          selector: DocumentSelector.t,
          extensionId: ExtensionId.t,
          displayName: string,
        })
      | RegisterRangeFormattingSupport({
          handle: int,
          selector: DocumentSelector.t,
          extensionId: ExtensionId.t,
          displayName: string,
        })
      | RegisterOnTypeFormattingSupport({
          handle: int,
          selector: DocumentSelector.t,
          autoFormatTriggerCharacters: list(string),
          extensionId: ExtensionId.t,
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

  module SCM: {
    [@deriving show]
    type msg =
      | RegisterSourceControl({
          handle: int,
          id: string,
          label: string,
          rootUri: option(Uri.t),
        })
      | UnregisterSourceControl({handle: int})
      | UpdateSourceControl({
          handle: int,
          hasQuickDiffProvider: option(bool),
          count: option(int),
          commitTemplate: option(string),
          acceptInputCommand: option(SCM.command),
        })
      // statusBarCommands: option(_),
      | RegisterSCMResourceGroup({
          provider: int,
          handle: int,
          id: string,
          label: string,
        })
      | UnregisterSCMResourceGroup({
          provider: int,
          handle: int,
        })
      | SpliceSCMResourceStates({
          handle: int,
          splices: list(SCM.Resource.Splices.t),
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
      | SendProcessReady({
          terminalId: int,
          pid: int,
          workingDirectory: string,
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
          label: Label.t,
          source: string,
          alignment,
          command: option(Command.t),
          priority: int,
        })
      | Dispose({id: int});
  };

  [@deriving show]
  type t =
    | Connected
    | Ready
    | Clipboard(Clipboard.msg)
    | Commands(Commands.msg)
    | DebugService(DebugService.msg)
    | Decorations(Decorations.msg)
    | Diagnostics(Diagnostics.msg)
    | DocumentContentProvider(DocumentContentProvider.msg)
    | ExtensionService(ExtensionService.msg)
    | FileSystem(FileSystem.msg)
    | LanguageFeatures(LanguageFeatures.msg)
    | MessageService(MessageService.msg)
    | SCM(SCM.msg)
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

module Reply: {
  type t;

  let none: t;

  let error: string => t;

  let okEmpty: t;

  let okJson: Yojson.Safe.t => t;
};

module Middleware: {
  type t = (Msg.t => Lwt.t(Reply.t), Msg.t) => Lwt.t(Reply.t);

  let filesystem:
    (
      ~stat: Uri.t => Lwt.t(Files.StatResult.t),
      ~readdir: Uri.t => Lwt.t(list((string, Files.FileType.t))),
      ~readFile: Uri.t => Lwt.t(Bytes.t),
      ~writeFile: (Uri.t, Bytes.t) => Lwt.t(unit),
      ~rename: (
                 ~source: Uri.t,
                 ~target: Uri.t,
                 Files.FileOverwriteOptions.t
               ) =>
               Lwt.t(unit),
      ~copy: (~source: Uri.t, ~target: Uri.t, Files.FileOverwriteOptions.t) =>
             Lwt.t(unit),
      ~mkdir: Uri.t => Lwt.t(unit),
      ~delete: (Uri.t, Files.FileDeleteOptions.t) => Lwt.t(unit)
    ) =>
    t;
};

module Client: {
  type t;

  let start:
    (
      ~initialConfiguration: Configuration.t=?,
      ~initialWorkspace: WorkspaceData.t=?,
      ~namedPipe: NamedPipe.t,
      ~initData: Extension.InitData.t,
      // TODO:
      // Is there a way to use GADT's to strongly-type the reply from the request?
      // Right now, we take arbitrary JSON responses, without help from the type
      // system that these are correct.
      ~handler: Msg.t => Lwt.t(Reply.t),
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

  module Decorations: {
    type request = {
      id: int,
      handle: int,
      uri: Uri.t,
    };

    type decoration = {
      priority: int,
      bubble: bool,
      title: string,
      letter: string,
      color: ThemeColor.t,
    };

    type reply = IntMap.t(decoration);

    let provideDecorations:
      (~requests: list(request), Client.t) => Lwt.t(reply);
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
      Lwt.t(list(DefinitionLink.t));

    let provideDeclaration:
      (
        ~handle: int,
        ~resource: Uri.t,
        ~position: OneBasedPosition.t,
        Client.t
      ) =>
      Lwt.t(list(DefinitionLink.t));

    let provideHover:
      (
        ~handle: int,
        ~resource: Uri.t,
        ~position: OneBasedPosition.t,
        Client.t
      ) =>
      Lwt.t(option(Hover.t));

    let provideImplementation:
      (
        ~handle: int,
        ~resource: Uri.t,
        ~position: OneBasedPosition.t,
        Client.t
      ) =>
      Lwt.t(list(DefinitionLink.t));

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
      Lwt.t(list(DefinitionLink.t));

    let provideSignatureHelp:
      (
        ~handle: int,
        ~resource: Uri.t,
        ~position: OneBasedPosition.t,
        ~context: SignatureHelp.RequestContext.t,
        Client.t
      ) =>
      Lwt.t(option(SignatureHelp.Response.t));

    let provideDocumentFormattingEdits:
      (
        ~handle: int,
        ~resource: Uri.t,
        ~options: FormattingOptions.t,
        Client.t
      ) =>
      Lwt.t(option(list(Edit.SingleEditOperation.t)));

    let provideDocumentRangeFormattingEdits:
      (
        ~handle: int,
        ~resource: Uri.t,
        ~range: OneBasedRange.t,
        ~options: FormattingOptions.t,
        Client.t
      ) =>
      Lwt.t(option(list(Edit.SingleEditOperation.t)));

    let provideOnTypeFormattingEdits:
      (
        ~handle: int,
        ~resource: Uri.t,
        ~position: OneBasedPosition.t,
        ~character: string,
        ~options: FormattingOptions.t,
        Client.t
      ) =>
      Lwt.t(option(list(Edit.SingleEditOperation.t)));

    let releaseSignatureHelp: (~handle: int, ~id: int, Client.t) => unit;
  };

  module SCM: {
    let provideOriginalResource:
      (~handle: int, ~uri: Uri.t, Client.t) => Lwt.t(option(Uri.t));

    let onInputBoxValueChange:
      (~handle: int, ~value: string, Client.t) => unit;
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
