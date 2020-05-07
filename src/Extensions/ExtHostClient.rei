module Core = Oni_Core;
module Protocol = ExtHostProtocol;
module Workspace = Protocol.Workspace;
module Configuration = Exthost.Configuration;

type t;

// SCM

module SCM: {
  // MODEL

  [@deriving show]
  type command = {
    id: string,
    title: string,
    tooltip: option(string),
    arguments: list(Core.Json.t),
  };

  module Resource: {
    [@deriving show]
    type t = {
      handle: int,
      uri: Core.Uri.t,
      icons: list(string),
      tooltip: string,
      strikeThrough: bool,
      faded: bool,
    };
  };

  module ResourceGroup: {
    [@deriving show]
    type t = {
      handle: int,
      id: string,
      label: string,
      hideWhenEmpty: bool,
      resources: list(Resource.t),
    };
  };

  module Provider: {
    [@deriving show]
    type t = {
      handle: int,
      id: string,
      label: string,
      rootUri: option(Core.Uri.t),
      resourceGroups: list(ResourceGroup.t),
      hasQuickDiffProvider: bool,
      count: int,
      commitTemplate: string,
      acceptInputCommand: option(command),
    };
  };

  // UPDATE

  type msg =
    | RegisterSourceControl({
        handle: int,
        id: string,
        label: string,
        rootUri: option(Core.Uri.t),
      })
    | UnregisterSourceControl({handle: int})
    | UpdateSourceControl({
        handle: int,
        hasQuickDiffProvider: option(bool),
        count: option(int),
        commitTemplate: option(string),
        acceptInputCommand: option(command),
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
        provider: int,
        group: int,
        start: int,
        deleteCount: int,
        additions: list(Resource.t),
      });

  let handleMessage:
    (~dispatch: msg => unit, string, list(Core.Json.t)) => unit;

  // EFFECTS

  module Effects: {
    let provideOriginalResource:
      (t, list(Provider.t), string, Core.Uri.t => 'msg) =>
      Isolinear.Effect.t('msg);

    let onInputBoxValueChange:
      (t, Provider.t, string) => Isolinear.Effect.t(_);
  };
};

// SCM

module Terminal: {
  // MODEL

  module ShellLaunchConfig: {
    [@deriving show({with_path: false})]
    type t = {
      name: string,
      executable: string,
      arguments: list(string),
    };
  };

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

  module Requests: {
    let createProcess:
      (int, ShellLaunchConfig.t, Core.Uri.t, int, int, t) => unit;

    let acceptProcessResize: (int, int, int, t) => unit;

    let acceptProcessInput: (int, string, t) => unit;

    let acceptProcessShutdown: (~immediate: bool=?, int, t) => unit;
  };
};

type msg =
  | SCM(SCM.msg)
  | Terminal(Terminal.msg)
  | ShowMessage({
      severity: [ | `Ignore | `Info | `Warning | `Error],
      message: string,
      extensionId: option(string),
    })
  | RegisterTextContentProvider({
      handle: int,
      scheme: string,
    })
  | UnregisterTextContentProvider({handle: int})
  | RegisterDecorationProvider({
      handle: int,
      label: string,
    })
  | UnregisterDecorationProvider({handle: int})
  | DecorationsDidChange({
      handle: int,
      uris: list(Core.Uri.t),
    });
