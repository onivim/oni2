open Oni_Core;

// MODEL

[@deriving show]
type command;

module Resource: {
  [@deriving show]
  type t = {
    handle: int,
    uri: Uri.t,
    //    icons: Exthost.SCM.Resource.Icons.t,
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
    (Exthost.Client.t, model, string, Uri.t => 'msg) =>
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

let update: (Exthost.Client.t, model, msg) => (model, outmsg);

let handleExtensionMessage:
  (~dispatch: msg => unit, Exthost.Msg.SCM.msg) => unit;

// VIEW

module Pane: {
  let make:
    (
      ~model: model,
      ~workingDirectory: string,
      ~onItemClick: Resource.t => unit,
      ~isFocused: bool,
      ~theme: ColorTheme.Colors.t,
      ~font: UiFont.t,
      ~dispatch: msg => unit,
      unit
    ) =>
    Revery.UI.element;
};
