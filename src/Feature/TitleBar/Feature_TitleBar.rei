open Oni_Core;

type windowDisplayMode =
  | Minimized
  | Windowed
  | Maximized
  | Fullscreen;

[@deriving show]
type msg =
  | WindowMinimizeClicked
  | WindowMaximizeClicked
  | WindowRestoreClicked
  | WindowCloseClicked
  | TitleDoubleClicked;

module View: {
  let make: (
    ~dispatch: msg => unit,
    ~isFocused: bool,
    ~windowDisplayMode: windowDisplayMode,
    ~title: string,
    ~theme: ColorTheme.Colors.t,
    ~font: UiFont.t,
    unit
  ) => Revery.UI.element;
};
