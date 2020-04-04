open Oni_Core;
open Oni_Extensions;

// MODEL

[@deriving show]
type command;

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

[@deriving show]
type model;

let initial: model;

// EFFECTS

module Effects: {
  let getOriginalUri:
    (ExtHostClient.t, model, string, Uri.t => 'msg) =>
    Isolinear.Effect.t('msg);
};

// UPDATE

[@deriving show]
type msg;

module Msg: {let keyPressed: string => msg;};

type outmsg =
  | Effect(Isolinear.Effect.t(msg))
  | Focus
  | Nothing;

let update: (ExtHostClient.t, model, msg) => (model, outmsg);

let handleExtensionMessage:
  (~dispatch: msg => unit, ExtHostClient.SCM.msg) => unit;

// VIEW

module Pane: {
  let make:
    (
      ~model: model,
      ~workingDirectory: option(string),
      ~onItemClick: Resource.t => unit,
      ~isFocused: bool,
      ~theme: ColorTheme.resolver,
      ~font: UiFont.t,
      ~dispatch: msg => unit,
      unit
    ) =>
    Revery.UI.element;
};
