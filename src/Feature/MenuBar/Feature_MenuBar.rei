open Oni_Core;

type model;

let initial: MenuBar.Schema.t => model;

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
      unit
    ) =>
    Revery.UI.element;
};
