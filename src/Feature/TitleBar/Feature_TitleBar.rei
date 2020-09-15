open Oni_Core;

// MODEL

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

let title:
  (
    ~activeBuffer: option(Oni_Core.Buffer.t),
    ~workspaceRoot: string,
    ~workspaceDirectory: string,
    ~config: Oni_Core.Config.resolver
  ) =>
  string;

// VIEW

module View: {
  let make:
    (
      ~activeBuffer: option(Oni_Core.Buffer.t),
      ~workspaceRoot: string,
      ~workspaceDirectory: string,
      ~config: Config.resolver,
      ~dispatch: msg => unit,
      ~isFocused: bool,
      ~windowDisplayMode: windowDisplayMode,
      ~theme: ColorTheme.Colors.t,
      ~font: UiFont.t,
      unit
    ) =>
    Revery.UI.element;
};

// CONTRIBUTIONS

module Contributions: {let configuration: list(Config.Schema.spec);};
