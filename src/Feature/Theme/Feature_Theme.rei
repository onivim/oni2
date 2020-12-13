open Oni_Core;

module Colors = GlobalColors;

type model;

type theme = Exthost.Extension.Contributions.Theme.t;

let initial: list(list(ColorTheme.Schema.definition)) => model;

[@deriving show]
type command;

[@deriving show]
type msg =
  | Command(command)
  | TextmateThemeLoaded(ColorTheme.variant, [@opaque] Textmate.ColorTheme.t);

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

module Commands: {let selectTheme: Command.t(msg);};

module Contributions: {let commands: list(Command.t(msg));};
