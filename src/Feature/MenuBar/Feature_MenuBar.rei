open Oni_Core;

type model;

let initial: model;

module Global = Global;

module View: {
  let make:
    (
      ~isWindowFocused: bool,
      ~theme: ColorTheme.Colors.t,
      ~font: UiFont.t,
      ~config: Config.resolver,
      ~model: model,
      unit
    ) =>
    Revery.UI.element;
};
