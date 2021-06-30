open Oni_Core;

[@deriving show]
type msg;

type model;

let initial:
  (
    ~menus: list(ContextMenu.Schema.menu),
    ~groups: list(ContextMenu.Schema.group)
  ) =>
  model;

type outmsg =
  | Nothing
  | ExecuteCommand({command: string});

let update:
  (
    ~contextKeys: WhenExpr.ContextKeys.t,
    ~commands: Command.Lookup.t(_),
    msg,
    model
  ) =>
  (model, outmsg);

// SUBSCRIPTION

let sub:
  (
    ~config: Config.resolver,
    ~contextKeys: WhenExpr.ContextKeys.t,
    ~commands: Command.Lookup.t(_),
    ~input: Feature_Input.model,
    model
  ) =>
  Isolinear.Sub.t(msg);

module Global = Global;

module View: {
  let make:
    (
      ~isWindowFocused: bool,
      ~theme: ColorTheme.Colors.t,
      ~font: UiFont.t,
      ~config: Config.resolver,
      ~context: WhenExpr.ContextKeys.t,
      ~input: Feature_Input.model,
      ~model: model,
      ~dispatch: msg => unit,
      unit
    ) =>
    Revery.UI.element;
};

// CONFIGURATION

module Configuration: {
  let visibility: Config.Schema.setting([ | `visible | `hidden]);
};

// CONTRIBUTIONS

module Contributions: {let configuration: list(Config.Schema.spec);};
