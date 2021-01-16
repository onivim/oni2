open Oni_Core;

// MODEL

type windowDisplayMode =
  | Minimized
  | Windowed
  | Maximized
  | Fullscreen;

[@deriving show]
type msg;

let title:
  (
    ~activeBuffer: option(Oni_Core.Buffer.t),
    ~workspaceRoot: string,
    ~workspaceDirectory: string,
    ~config: Oni_Core.Config.resolver
  ) =>
  string;

// UPDATE

type outmsg =
  | Nothing
  | Effect(Isolinear.Effect.t(msg));

let update:
  (
    ~maximize: unit => unit,
    ~minimize: unit => unit,
    ~restore: unit => unit,
    ~close: unit => unit,
    msg
  ) =>
  outmsg;

// VIEW

module View: {
  let make:
    (
      ~menuBar: Revery.UI.element,
      ~activeBuffer: option(Oni_Core.Buffer.t),
      ~workspaceRoot: string,
      ~workspaceDirectory: string,
      ~registration: Feature_Registration.model,
      ~config: Config.resolver,
      ~dispatch: msg => unit,
      ~registrationDispatch: Feature_Registration.msg => unit,
      ~isFocused: bool,
      ~windowDisplayMode: windowDisplayMode,
      ~theme: ColorTheme.Colors.t,
      ~font: UiFont.t,
      ~height: float,
      unit
    ) =>
    Revery.UI.element;
};

// CONTRIBUTIONS

module Contributions: {let configuration: list(Config.Schema.spec);};
