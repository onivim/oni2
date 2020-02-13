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

[@deriving show]
type model;

let initial: model;

// EFFECTS

module Effects: {
  let getOriginalUri:
    (Oni_Extensions.ExtHostClient.t, model, string, Uri.t => 'msg) =>
    Isolinear.Effect.t('msg);
};

// UPDATE

[@deriving show]
type msg;

let update: (msg, model) => (model, Isolinear.Effect.t(msg));

let handleExtensionMessage:
  (~dispatch: msg => unit, Oni_Extensions.SCM.msg) => unit;

// VIEW

module Pane: {
  let make:
    (
      ~model: model,
      ~workingDirectory: option(string),
      ~onItemClick: Resource.t => unit,
      ~theme: Theme.t,
      ~font: UiFont.t,
      unit
    ) =>
    Revery.UI.element;
};
