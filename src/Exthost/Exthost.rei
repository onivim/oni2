open EditorCoreTypes;
open Oni_Core;

module Extension = Exthost_Extension;
module Protocol = Exthost_Protocol;
module Transport = Exthost_Transport;

module ChainedCacheId: {
  [@deriving show]
  type t;
};

module Label: {
  [@deriving show]
  type segment =
    | Text(string)
    | Icon(string);

  [@deriving show]
  type t;

  let segments: t => list(segment);

  let ofString: string => t;
  let toString: t => string;

  let decode: Json.decoder(t);
};

module Command: {
  [@deriving show]
  type t = {
    id: option(string),
    label: option(Label.t),
  };

  let decode: Json.decoder(t);
};

module CompletionContext: {
  [@deriving show]
  type triggerKind =
    | Invoke
    | TriggerCharacter
    | TriggerForIncompleteCompletions;

  [@deriving show]
  type t = {
    triggerKind,
    triggerCharacter: option(string),
  };
};

module CompletionKind: {
  [@deriving show]
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

  let ofRange: CharacterRange.t => t;
  let toRange: t => CharacterRange.t;
};

module CodeLens: {
  [@deriving show]
  type lens = {
    cacheId: option(list(int)),
    range: OneBasedRange.t,
    command: option(Command.t),
  };

  module List: {
    [@deriving show]
    type cacheId;

    [@deriving show]
    type t = {
      cacheId: option(cacheId),
      lenses: list(lens),
    };

    let default: t;

    let decode: Json.decoder(t);
  };
};

module Location: {
  type t = {
    uri: Uri.t,
    range: OneBasedRange.t,
  };

  let decode: Json.decoder(t);
};

module MarkdownString: {
  [@deriving show]
  type t;

  let fromString: string => t;
  let toString: t => string;

  let decode: Json.decoder(t);
};

module Edit: {
  module SingleEditOperation: {
    [@deriving show]
    type t = {
      range: OneBasedRange.t,
      text: option(string),
      forceMoveMarkers: bool,
    };

    // Get the difference in lines due to the edit
    let deltaLineCount: t => int;

    let decode: Json.decoder(t);
  };
};

module ExtensionActivationError: {
  [@deriving show]
  type t =
    | Message(string)
    | MissingDependency(ExtensionId.t);

  let toString: t => string;
};

module ExtensionActivationReason: {
  type t;

  let create:
    (~startup: bool, ~extensionId: string, ~activationEvent: string) => t;

  let encode: Json.encoder(t);
};

module ExtensionId: {
  [@deriving show]
  type t = string;

  let toString: t => string;

  let decode: Json.decoder(t);
};

module DefinitionLink: {
  [@deriving show]
  type t = {
    uri: Uri.t,
    range: OneBasedRange.t,
    originSelectionRange: option(OneBasedRange.t),
    targetSelectionRange: option(OneBasedRange.t),
  };

  let decode: Json.decoder(t);
};

module DocumentSelector: {
  [@deriving show]
  type t = list(DocumentFilter.t);

  let matchesBuffer: (~buffer: Oni_Core.Buffer.t, t) => bool;

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

module Message: {
  [@deriving show]
  type severity =
    | Ignore
    | Info
    | Warning
    | Error;

  [@deriving show]
  type handle;

  let handleToJson: handle => Yojson.Safe.t;

  module Command: {
    [@deriving show]
    type t = {
      title: string,
      isCloseAffordance: bool,
      handle,
    };

    let decode: Json.decoder(t);
  };
};

module RenameLocation: {
  type t = {
    range: OneBasedRange.t,
    text: string,
  };

  let decode: Json.decoder(t);
};

module SuggestItem: {
  module InsertTextRules: {
    [@deriving show]
    type rule =
      | KeepWhitespace // 0b001
      | InsertAsSnippet; // 0b100

    [@deriving show]
    type t;

    let none: t;

    let insertAsSnippet: t;

    let matches: (~rule: rule, t) => bool;
  };
  module SuggestRange: {
    [@deriving show]
    type t =
      | Single(OneBasedRange.t)
      | Combo({
          insert: OneBasedRange.t,
          replace: OneBasedRange.t,
        });
  };

  [@deriving show]
  type t = {
    chainedCacheId: option(ChainedCacheId.t),
    label: string,
    kind: CompletionKind.t,
    detail: option(string),
    documentation: option(MarkdownString.t),
    sortText: option(string),
    filterText: option(string),
    insertText: option(string),
    insertTextRules: InsertTextRules.t,
    suggestRange: option(SuggestRange.t),
    commitCharacters: list(string),
    additionalTextEdits: list(Edit.SingleEditOperation.t),
    command: option(Command.t),
  };

  let insertText: t => string;
  let filterText: t => string;
  let sortText: t => string;
};

module ReferenceContext: {
  type t = {includeDeclaration: bool};

  let encode: Json.encoder(t);
};

module Progress: {
  module Location: {
    [@deriving show]
    type t =
      | Explorer
      | SCM
      | Extensions
      | Window
      | Notification
      | Dialog
      | Other(string);

    let decode: Json.decoder(t);
  };

  module Options: {
    [@deriving show]
    type t = {
      location: Location.t,
      title: option(string),
      source: option(string),
      total: option(int),
      cancellable: bool,
      buttons: list(string),
    };

    let decode: Json.decoder(t);
  };

  module Step: {
    [@deriving show]
    type t = {
      message: option(string),
      increment: option(int),
      total: option(int),
    };

    let decode: Json.decoder(t);
  };
};

module InputBoxOptions: {
  [@deriving show]
  type t = {
    ignoreFocusOut: bool,
    password: bool,
    placeHolder: option(string),
    prompt: option(string),
    value: option(string),
    // TODO
    // valueSelection: (int, int),
  };
};

module QuickOpen: {
  module Options: {
    [@deriving show]
    type t = {
      placeholder: option(string),
      matchOnDescription: bool,
      matchOnDetail: bool,
      matchOnLabel: bool, // default true
      autoFocusOnList: bool, // default true
      ignoreFocusLost: bool,
      canPickMany: bool,
      // TODO:
      // https://github.com/onivim/vscode-exthost/blob/a25f426a04fe427beab7465be660f89a794605b5/src/vs/platform/quickinput/common/quickInput.ts#L78
      //quickNavigate
      contextKey: option(string),
    };
  };

  module Button: {
    [@deriving show]
    type icon = {
      dark: Oni_Core.Uri.t,
      light: option(Oni_Core.Uri.t),
    };

    [@deriving show]
    type t = {
      iconPath: option(icon),
      iconClass: option(string),
      tooltip: option(string),
    };
  };

  module Item: {
    [@deriving show]
    type t = {
      // TransferQuickPickItems
      handle: int,
      // IQuickPickItem
      id: option(string),
      label: string,
      description: option(string),
      detail: option(string),
      iconClasses: list(string),
      buttons: list(Button.t),
      picked: bool,
      alwaysShow: bool,
    };
  };

  module QuickPick: {
    [@deriving show]
    type t = {
      // BaseTransferQuickInput
      id: int,
      enabled: bool,
      busy: bool,
      visible: bool,
      // TransferQuickPick
      value: option(string),
      placeholder: option(string),
      buttons: list(Button.t),
      items: list(Item.t),
      // TODO:
      // activeItems
      // selectedItems
      // canSelectMany
      // ignoreFocusOut
      // matchOnDescription
      // Match on Detail
    };
  };

  module QuickInput: {
    [@deriving show]
    type t = {
      // BaseTransferQuickInput
      id: int,
      enabled: bool,
      busy: bool,
      visible: bool,
      // TransferInputBox
      value: option(string),
      placeholder: option(string),
      buttons: list(Button.t),
      prompt: option(string),
      validationMessage: option(string),
    };
  };

  [@deriving show]
  type t =
    | QuickPick(QuickPick.t)
    | QuickInput(QuickInput.t);
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
      type t = {
        handle: int,
        resourceSplices: [@opaque] list(Splice.t),
      };
    };

    module Decode: {let splices: Json.decoder(Splices.t);};
  };

  module ProviderFeatures: {
    type t = {
      hasQuickDiffProvider: bool,
      count: option(int),
      commitTemplate: option(string),
      acceptInputCommand: option(command),
      statusBarCommands: option(list(command)),
    };

    let decode: Json.decoder(t);
  };

  module GroupFeatures: {
    [@deriving show({with_path: false})]
    type t = {hideWhenEmpty: bool};

    let decode: Json.decoder(t);
  };

  module Group: {
    [@deriving show({with_path: false})]
    type t = {
      handle: int,
      id: string,
      label: string,
      features: GroupFeatures.t,
    };

    let decode: Json.decoder(t);
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
      label: [ | `String(string) | `Range(int, int)],
      documentation: option(MarkdownString.t),
    };
    let decode: Json.decoder(t);
  };

  module Signature: {
    [@deriving show]
    type t = {
      label: string,
      documentation: option(MarkdownString.t),
      parameters: list(ParameterInformation.t),
    };

    let decode: Json.decoder(t);
  };

  type cacheId;

  module Response: {
    type t = {
      cacheId,
      signatures: list(Signature.t),
      activeSignature: int,
      activeParameter: int,
    };

    let decode: Json.decoder(t);
  };
};

module SuggestResult: {
  [@deriving show];

  type cacheId;

  [@deriving show]
  type t = {
    completions: list(SuggestItem.t),
    isIncomplete: bool,
    cacheId: option(cacheId),
  };

  let empty: t;
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

  module Target: {
    type t =
      | User
      | UserLocal
      | UserRemote
      | Workspace
      | WorkspaceFolder
      | Default
      | Memory;

    let toInt: t => int;
    let ofInt: int => option(t);
    let toString: t => string;

    let encode: Json.encoder(t);
    let decode: Json.decoder(t);
  };

  module Overrides: {
    type t = {
      overrideIdentifier: option(string),
      resource: option(Oni_Core.Uri.t),
    };

    let decode: Json.decoder(t);
  };

  type t;

  let to_yojson: t => Json.t;
  let encode: Json.encoder(t);
  let empty: t;
  let create:
    (~defaults: Model.t=?, ~user: Model.t=?, ~workspace: Model.t=?, unit) => t;
};

module Diagnostic: {
  module Severity: {
    [@deriving show]
    type t =
      | Hint
      | Info
      | Warning
      | Error;

    let toInt: t => int;
    let ofInt: int => option(t);
    let max: (t, t) => t;
  };
  type t = {
    range: OneBasedRange.t,
    message: string,
    severity: Severity.t,
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

  module ReadDirResult: {
    type t = (string, FileType.t);

    let encode: Json.encoder(t);
  };

  module FileSystemEvents: {
    type t = {
      created: list(Oni_Core.Uri.t),
      changed: list(Oni_Core.Uri.t),
      deleted: list(Oni_Core.Uri.t),
    };

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

  let ofPosition: EditorCoreTypes.CharacterPosition.t => t;
  let to_yojson: t => Yojson.Safe.t;
};

module ModelContentChange: {
  type t = {
    range: OneBasedRange.t,
    text: string,
    rangeLength: int,
  };

  let ofBufferUpdate:
    (~previousBuffer: Oni_Core.Buffer.t, BufferUpdate.t, Eol.t) => t;

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
  type environment =
    | Inherit
    | Additive(StringMap.t(string))
    | Strict(StringMap.t(string));

  type t = {
    name: string,
    executable: string,
    arguments: list(string),
    env: environment,
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

  let fromUri: (~name: string, Uri.t) => t;
  let fromPath: string => t;

  let encode: Json.encoder(t);
  let decode: Json.decoder(t);
};

module ThemeColor: {
  [@deriving show]
  type t = {id: string};

  let decode: Json.decoder(t);
};

module Color: {
  [@deriving show]
  type t;

  let decode: Json.decoder(t);

  let resolve: (Oni_Core.ColorTheme.Colors.t, t) => option(Revery.Color.t);
};

module WorkspaceEdit: {
  module FileEdit: {type t;};

  module TextEdit: {type t;};

  type edit =
    | File(FileEdit.t)
    | Text(TextEdit.t);

  type t = {
    edits: list(edit),
    rejectReason: option(string),
  };
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

  module Configuration: {
    [@deriving show]
    type msg =
      | UpdateConfigurationOption({
          target: option(Configuration.Target.t),
          key: string,
          value: Yojson.Safe.t,
          overrides: option(Configuration.Overrides.t),
          scopeToLanguage: bool,
        })
      | RemoveConfigurationOption({
          target: option(Configuration.Target.t),
          key: string,
          overrides: option(Configuration.Overrides.t),
          scopeToLanguage: bool,
        });
  };

  module Console: {
    [@deriving show]
    type msg =
      | LogExtensionHostMessage({
          logType: string,
          severity: string,
          arguments: Yojson.Safe.t,
        });
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

  module Documents: {
    [@deriving show]
    type msg =
      | TryCreateDocument({
          language: option(string),
          content: option(string),
        })
      | TryOpenDocument({uri: Oni_Core.Uri.t})
      | TrySaveDocument({uri: Oni_Core.Uri.t});
  };

  module DownloadService: {
    [@deriving show]
    type msg =
      | Download({
          uri: Oni_Core.Uri.t,
          dest: Oni_Core.Uri.t,
        });
  };

  module Errors: {
    [@deriving show]
    type msg =
      | OnUnexpectedError(Yojson.Safe.t);
  };

  module ExtensionService: {
    [@deriving show]
    type msg =
      | ActivateExtension({
          extensionId: ExtensionId.t,
          activationEvent: option(string),
        })
      | WillActivateExtension({extensionId: ExtensionId.t})
      | DidActivateExtension({
          extensionId: ExtensionId.t,
          //startup: bool,
          codeLoadingTime: int,
          activateCallTime: int,
          activateResolvedTime: int,
        })
      | ExtensionActivationError({
          extensionId: ExtensionId.t,
          error: ExtensionActivationError.t,
        })
      | ExtensionRuntimeError({
          extensionId: ExtensionId.t,
          errorsJson: list(Yojson.Safe.t),
        });
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
      | EmitCodeLensEvent({
          eventHandle: int,
          event: Yojson.Safe.t,
        }) // ??
      | RegisterCodeLensSupport({
          handle: int,
          selector: DocumentSelector.t,
          eventHandle: option(int),
        })
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
      | RegisterRenameSupport({
          handle: int,
          selector: DocumentSelector.t,
          supportsResolveInitialValues: bool,
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

  module Languages: {
    [@deriving show]
    type msg =
      | GetLanguages
      | ChangeLanguage({
          uri: Oni_Core.Uri.t,
          languageId: string,
        });
  };

  module MessageService: {
    [@deriving show]
    type msg =
      | ShowMessage({
          severity: Message.severity,
          message: string,
          extensionId: option(string),
          commands: list(Message.Command.t),
        });
  };

  module Progress: {
    [@deriving show]
    type msg =
      | StartProgress({
          handle: int,
          options: Progress.Options.t,
        })
      | ProgressReport({
          handle: int,
          message: Progress.Step.t,
        })
      | ProgressEnd({handle: int});
  };

  module QuickOpen: {
    [@deriving show]
    type msg =
      // TODO: How to handle incoming cancellation token?
      | Show({
          instance: int,
          options: QuickOpen.Options.t,
        }) // Returns a promise / id
      | SetItems({
          instance: int,
          items: list(QuickOpen.Item.t),
        })
      | SetError({
          instance: int,
          error: Yojson.Safe.t,
        })
      | Input({
          options: InputBoxOptions.t,
          validateInput: bool,
        })
      | CreateOrUpdate({params: QuickOpen.t})
      | Dispose({id: int});
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
          features: SCM.ProviderFeatures.t,
        })
      | RegisterSCMResourceGroups({
          provider: int,
          groups: list(SCM.Group.t),
          splices: [@opaque] list(SCM.Resource.Splices.t),
        })
      | UnregisterSCMResourceGroup({
          provider: int,
          handle: int,
        })
      | UpdateGroup({
          provider: int,
          handle: int,
          features: SCM.GroupFeatures.t,
        })
      | UpdateGroupLabel({
          provider: int,
          handle: int,
          label: string,
        })
      | SetInputBoxPlaceholder({
          handle: int,
          value: string,
        })
      | SetInputBoxVisibility({
          handle: int,
          visible: bool,
        })
      | SetValidationProviderIsEnabled({
          handle: int,
          enabled: bool,
        })
      | SpliceSCMResourceStates({
          handle: int,
          splices: list(SCM.Resource.Splices.t),
        });
  };

  module Storage: {
    [@deriving show]
    type msg =
      | GetValue({
          shared: bool,
          key: string,
        })
      | SetValue({
          shared: bool,
          key: string,
          value: Yojson.Safe.t,
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

  module OutputService: {
    [@deriving show]
    type msg =
      | Register({
          label: string,
          log: bool,
          file: option(Oni_Core.Uri.t),
        })
      | Append({
          channelId: string,
          value: string,
        })
      | Update({channelId: string})
      | Clear({
          channelId: string,
          till: int,
        })
      | Reveal({
          channelId: string,
          preserveFocus: bool,
        })
      | Close({channelId: string})
      | Dispose({channelId: string});
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
          color: option(Color.t),
          backgroundColor: option(Color.t),
          tooltip: option(string),
          priority: int,
        })
      | Dispose({id: int});
  };

  module Window: {
    [@deriving show]
    type msg =
      | GetWindowVisibility
      | OpenUri({uri: Oni_Core.Uri.t});
  };

  module Workspace: {
    [@deriving show]
    type msg =
      | SaveAll({includeUntitled: bool})
      | StartFileSearch({
          includePattern: option(string),
          //        includeFolder: option(Oni_Core.Uri.t),
          excludePattern: option(string),
          maxResults: option(int),
        });
  };

  [@deriving show]
  type t =
    | Connected
    | Ready
    | Clipboard(Clipboard.msg)
    | Commands(Commands.msg)
    | Configuration(Configuration.msg)
    | Console(Console.msg)
    | DebugService(DebugService.msg)
    | Decorations(Decorations.msg)
    | Diagnostics(Diagnostics.msg)
    | DocumentContentProvider(DocumentContentProvider.msg)
    | Documents(Documents.msg)
    | DownloadService(DownloadService.msg)
    | Errors(Errors.msg)
    | ExtensionService(ExtensionService.msg)
    | FileSystem(FileSystem.msg)
    | LanguageFeatures(LanguageFeatures.msg)
    | Languages(Languages.msg)
    | MessageService(MessageService.msg)
    | OutputService(OutputService.msg)
    | Progress(Progress.msg)
    | QuickOpen(QuickOpen.msg)
    | SCM(SCM.msg)
    | StatusBar(StatusBar.msg)
    | Storage(Storage.msg)
    | Telemetry(Telemetry.msg)
    | TerminalService(TerminalService.msg)
    | Window(Window.msg)
    | Workspace(Workspace.msg)
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

  let okBuffer: Bytes.t => t;
};

module Middleware: {let download: Msg.DownloadService.msg => Lwt.t(Reply.t);};

module Client: {
  type t;

  let start:
    (
      ~initialConfiguration: Configuration.t=?,
      ~initialWorkspace: option(WorkspaceData.t)=?,
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
      uri: Uri.t,
    };

    type decoration = {
      bubble: bool,
      title: string,
      letter: string,
      color: ThemeColor.t,
    };

    type reply = IntMap.t(decoration);

    let provideDecorations:
      (~handle: int, ~requests: list(request), Client.t) => Lwt.t(reply);
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
    let activateByEvent: (~event: string, Client.t) => Lwt.t(unit);

    let activate:
      (~extensionId: string, ~reason: ExtensionActivationReason.t, Client.t) =>
      Lwt.t(bool);

    let deltaExtensions:
      (
        ~toAdd: list(Exthost_Extension.InitData.Extension.t),
        ~toRemove: list(ExtensionId.t),
        Client.t
      ) =>
      Lwt.t(unit);
  };

  module FileSystem: {
    let readFile: (~handle: int, ~uri: Uri.t, Client.t) => Lwt.t(string);
  };

  module FileSystemEventService: {
    let onFileEvent: (~events: Files.FileSystemEvents.t, Client.t) => unit;
    // TODO
    // - onWillRunFileOperation
    // - onDidRunFileOperation
  };

  module LanguageFeatures: {
    let provideCodeLenses:
      (~handle: int, ~resource: Uri.t, Client.t) =>
      Lwt.t(option(CodeLens.List.t));

    let resolveCodeLens:
      (~handle: int, ~codeLens: CodeLens.lens, Client.t) =>
      Lwt.t(option(CodeLens.lens));

    let releaseCodeLenses:
      (~handle: int, ~cacheId: CodeLens.List.cacheId, Client.t) => unit;

    let provideCompletionItems:
      (
        ~handle: int,
        ~resource: Uri.t,
        ~position: OneBasedPosition.t,
        ~context: CompletionContext.t,
        Client.t
      ) =>
      Lwt.t(SuggestResult.t);

    let resolveCompletionItem:
      (~handle: int, ~chainedCacheId: ChainedCacheId.t, Client.t) =>
      Lwt.t(SuggestItem.t);

    let releaseCompletionItems:
      (~handle: int, ~cacheId: SuggestResult.cacheId, Client.t) => unit;

    let provideDocumentHighlights:
      (
        ~handle: int,
        ~resource: Uri.t,
        ~position: OneBasedPosition.t,
        Client.t
      ) =>
      Lwt.t(option(list(DocumentHighlight.t)));

    let provideDocumentSymbols:
      (~handle: int, ~resource: Uri.t, Client.t) =>
      Lwt.t(option(list(DocumentSymbol.t)));

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

    let provideRenameEdits:
      (
        ~handle: int,
        ~resource: Uri.t,
        ~position: OneBasedPosition.t,
        ~newName: string,
        Client.t
      ) =>
      Lwt.t(option(WorkspaceEdit.t));

    let resolveRenameLocation:
      (
        ~handle: int,
        ~resource: Uri.t,
        ~position: OneBasedPosition.t,
        Client.t
      ) =>
      Lwt.t(option(RenameLocation.t));

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

    let releaseSignatureHelp:
      (~handle: int, ~cacheId: SignatureHelp.cacheId, Client.t) => unit;

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

module GrammarInfo = GrammarInfo;
module LanguageInfo = LanguageInfo;
