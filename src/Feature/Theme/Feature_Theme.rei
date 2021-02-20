open Oni_Core;

module Colors = GlobalColors;

type model;

type theme = Exthost.Extension.Contributions.Theme.t;

let initial: list(list(ColorTheme.Schema.definition)) => model;

let variant: model => Oni_Core.ColorTheme.variant;

[@deriving show]
type command;

[@deriving show]
type msg;

module Msg: {let openThemePicker: msg;};

type outmsg =
  | Nothing
  | OpenThemePicker(list(theme))
  | ThemeChanged(ColorTheme.Colors.t)
  | NotifyError(string);

let update: (model, msg) => (model, outmsg);

let setTheme: (~themeId: string, model) => model;

let configurationChanged: (~resolver: Config.resolver, model) => model;

let colors:
  (
    ~extensionDefaults: list(ColorTheme.Defaults.t)=?,
    ~customizations: ColorTheme.Colors.t=?,
    model
  ) =>
  ColorTheme.Colors.t;

let tokenColors: model => Oni_Syntax.TokenTheme.t;

// SUBSCRIPTION

let sub:
  (
    ~getThemeContribution: string =>
                           option(Exthost.Extension.Contributions.Theme.t),
    model
  ) =>
  Isolinear.Sub.t(msg);

module Commands: {let selectTheme: Command.t(msg);};

module Contributions: {
  let commands: list(Command.t(msg));
  let configuration: list(Config.Schema.spec);
};
