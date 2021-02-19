open Oni_Core;

module Colors = GlobalColors;

type model;

type theme = Exthost.Extension.Contributions.Theme.t;

let initial: list(list(ColorTheme.Schema.definition)) => model;

let variant: model => Oni_Core.ColorTheme.variant;

[@deriving show]
type command;

[@deriving show]
type msg =
  | Command(command)
  | TextmateThemeLoaded({
      variant: ColorTheme.variant,
      colors: [@opaque] Textmate.ColorTheme.t,
      tokenColors: [@opaque] Oni_Syntax.TokenTheme.t,
    });

module Msg: {let openThemePicker: msg;};

type outmsg =
  | Nothing
  | OpenThemePicker(list(theme))
  | ThemeChanged(ColorTheme.Colors.t);

let update: (model, msg) => (model, outmsg);

let colors:
  (
    ~extensionDefaults: list(ColorTheme.Defaults.t)=?,
    ~customizations: ColorTheme.Colors.t=?,
    model
  ) =>
  ColorTheme.Colors.t;

let tokenColors: model => Oni_Syntax.TokenTheme.t;

module Commands: {let selectTheme: Command.t(msg);};

module Contributions: {
  let commands: list(Command.t(msg));
  let configuration: list(Config.Schema.spec);
};
