open Oni_Core;

[@deriving show]
type msg;

type model;

let initial: MenuBar.Schema.t => model;

let update: (msg, model) => model;

module Global = Global;

module View: {
  let make:
    (
      ~isWindowFocused: bool,
      ~theme: ColorTheme.Colors.t,
      ~font: UiFont.t,
      ~config: Config.resolver,
      ~contextKeys: WhenExpr.ContextKeys.t,
      ~commands: Command.Lookup.t(_),
      ~model: model,
      ~dispatch: msg => unit,
      unit
    ) =>
    Revery.UI.element;
};
