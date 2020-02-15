open Oni_Core;

// MODEL

[@deriving show]
type command = {
  id: string,
  title: string,
  arguments: list(Json.t),
};

module Resource: {
  [@deriving show]
  type t = {
    handle: int,
    uri: Uri.t,
    icons: list(string),
    tooltip: string,
    strikeThrough: bool,
    faded: bool,
    source: option(string),
    letter: option(string),
    color: option(string),
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
    rootUri: option(Uri.t),
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
      rootUri: option(Uri.t),
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

let handleMessage: (~dispatch: msg => unit, string, list(Json.t)) => unit;

// EFFECTS

module Effects: {
  let provideOriginalResource:
    (ExtHostTransport.t, list(Provider.t), string, Uri.t => 'msg) =>
    Isolinear.Effect.t('msg);

  let onInputBoxValueChange:
    (ExtHostTransport.t, Provider.t, string) => Isolinear.Effect.t(_);
};
