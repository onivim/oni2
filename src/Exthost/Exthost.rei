open Oni_Core;

module Extension = Exthost_Extension;
module Protocol = Exthost_Protocol;
module Transport = Exthost_Transport;

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

  let create: (
    ~versionId: int=?,
    ~lines: list(string)=?,
    ~eol: Eol.t =?,
    ~isDirty: bool=?,
    ~modeId: string,
    Uri.t
  ) => t;

  let to_yojson: t => Yojson.Safe.t;
};

module DocumentsAndEditorsDelta: {
  type t = {
    removedDocuments: list(Uri.t),
    addedDocuments: list(ModelAddedDelta.t),
    removedEditors: list(string),
    addedEditors: list(string),
  };

  let create: (
    ~removedDocuments: list(Uri.t),
    ~addedDocuments: list(ModelAddedDelta.t)
  ) => t;

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

module Msg: {
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
};

module Request: {
  module Commands: {
    let executeContributedCommand:
      (~arguments: list(Json.t), ~command: string, Client.t) => unit;
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

  module ExtensionService: {
    let activateByEvent: (~event: string, Client.t) => unit;
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
};
