open Oni_Core;

// MODEL

module Resource: {
  [@deriving show({with_path: false})]
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
  [@deriving show({with_path: false})]
  type t = {
    handle: int,
    id: string,
    label: string,
    hideWhenEmpty: bool,
    resources: list(Resource.t),
  };
};

module Provider: {
  [@deriving show({with_path: false})]
  type t = {
    handle: int,
    id: string,
    label: string,
    rootUri: option(Uri.t),
    resourceGroups: list(ResourceGroup.t),
    hasQuickDiffProvider: bool,
    count: int,
    commitTemplate: string,
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
    })
  // acceptInputCommand: option(_),
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

// REQUESTS

let provideOriginalResource: (int, Uri.t, ExtHostTransport.t) => Lwt.t(Uri.t);
