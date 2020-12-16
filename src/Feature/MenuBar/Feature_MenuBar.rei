open Oni_Core;

[@deriving show]
type msg;

type model;

let initial:
  (~menus: list(MenuBar.Schema.menu), ~items: list(MenuBar.Schema.item)) =>
  model;

let update:
  (
    ~contextKeys: WhenExpr.ContextKeys.t,
    ~commands: Command.Lookup.t(_),
    msg,
    model
  ) =>
  model;

module Global = Global;

module View: {
  let make:
    (
      ~isWindowFocused: bool,
      ~theme: ColorTheme.Colors.t,
      ~font: UiFont.t,
      ~config: Config.resolver,
      ~model: model,
      ~dispatch: msg => unit,
      unit
    ) =>
    Revery.UI.element;
};
