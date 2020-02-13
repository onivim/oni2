open Oni_Core;

// MODEL

module Resource: {
  [@deriving show({with_path: false})]
  type t =
    Oni_Extensions.SCM.Resource.t = {
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
  type t =
    Oni_Extensions.SCM.ResourceGroup.t = {
      handle: int,
      id: string,
      label: string,
      hideWhenEmpty: bool,
      resources: list(Resource.t),
    };
};

module Provider: {
  [@deriving show({with_path: false})]
  type t =
    Oni_Extensions.SCM.Provider.t = {
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

[@deriving show]
type t = {providers: list(Provider.t)};

let initial: t;

// EFFECTS

module Effects: {
  let getOriginalUri:
    (Oni_Extensions.ExtHostClient.t, t, string, Uri.t => 'msg) =>
    Isolinear.Effect.t('msg);
};

// UPDATE

[@deriving show]
type msg =
  | NewProvider({
      handle: int,
      id: string,
      label: string,
      rootUri: option(Uri.t),
    })
  | LostProvider({handle: int})
  | NewResourceGroup({
      provider: int,
      handle: int,
      id: string,
      label: string,
    })
  | LostResourceGroup({
      provider: int,
      handle: int,
    })
  | ResourceStatesChanged({
      provider: int,
      group: int,
      spliceStart: int,
      deleteCount: int,
      additions: list(Resource.t),
    })
  | CountChanged({
      handle: int,
      count: int,
    })
  | QuickDiffProviderChanged({
      handle: int,
      available: bool,
    })
  | CommitTemplateChanged({
      handle: int,
      template: string,
    });

let update: (msg, t) => (t, Isolinear.Effect.t(msg));

// VIEW

module Pane: {
  let make:
    (
      ~model: t,
      ~workingDirectory: option(string),
      ~onItemClick: Resource.t => unit,
      ~theme: Theme.t,
      ~font: UiFont.t,
      unit
    ) =>
    Revery.UI.element;
};
