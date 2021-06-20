open Oni_Core;

// MODEL

module Terminal: {
  type t;

  let id: t => int;
  let pid: t => option(int);
  let title: t => option(string);
  let launchConfig: t => Exthost.ShellLaunchConfig.t;
};

type t;

let initial: t;

let toList: t => list(Terminal.t);

let getTerminalOpt: (int, t) => option(Terminal.t);

// Font to be used for terminals
let font: t => Service_Font.font;

let configurationChanged: (~config: Config.resolver, t) => t;

// UPDATE

[@deriving show({with_path: false})]
type msg;

module Msg: {
  let terminalCreatedFromVim:
    (
      ~cmd: option(string),
      ~splitDirection: SplitDirection.t,
      ~closeOnExit: bool
    ) =>
    msg;

  let keyPressed: (~id: int, string) => msg;

  let pasted: (~id: int, string) => msg;
};

type outmsg =
  | Nothing
  | Effect(Isolinear.Effect.t(msg))
  | NotifyError(string)
  | SwitchToNormalMode
  | ClosePane({paneId: string})
  | TogglePane({paneId: string})
  | TerminalCreated({
      name: string,
      splitDirection: SplitDirection.t,
    })
  | TerminalExit({
      terminalId: int,
      exitCode: int,
      shouldClose: bool,
    });

let update:
  (
    ~clientServer: Feature_ClientServer.model,
    ~config: Config.resolver,
    t,
    msg
  ) =>
  (t, outmsg);

let subscription:
  (
    ~defaultFontFamily: string,
    ~defaultFontSize: float,
    ~defaultFontWeight: Revery.Font.Weight.t,
    ~defaultLigatures: FontLigatures.t,
    ~defaultSmoothing: FontSmoothing.t,
    ~config: Oni_Core.Config.resolver,
    ~setup: Setup.t,
    ~workspaceUri: Uri.t,
    Exthost.Client.t,
    t
  ) =>
  Isolinear.Sub.t(msg);

// let shellCmd: string;

// COLORS

module Colors: {
  let background: ColorTheme.Schema.definition;
  let foreground: ColorTheme.Schema.definition;
  let ansiBlack: ColorTheme.Schema.definition;
  let ansiRed: ColorTheme.Schema.definition;
  let ansiGreen: ColorTheme.Schema.definition;
  let ansiYellow: ColorTheme.Schema.definition;
  let ansiBlue: ColorTheme.Schema.definition;
  let ansiMagenta: ColorTheme.Schema.definition;
  let ansiCyan: ColorTheme.Schema.definition;
  let ansiWhite: ColorTheme.Schema.definition;
  let ansiBrightBlack: ColorTheme.Schema.definition;
  let ansiBrightRed: ColorTheme.Schema.definition;
  let ansiBrightGreen: ColorTheme.Schema.definition;
  let ansiBrightYellow: ColorTheme.Schema.definition;
  let ansiBrightBlue: ColorTheme.Schema.definition;
  let ansiBrightMagenta: ColorTheme.Schema.definition;
  let ansiBrightCyan: ColorTheme.Schema.definition;
  let ansiBrightWhite: ColorTheme.Schema.definition;
};

let theme: ColorTheme.Colors.t => EditorTerminal.Theme.t;
let defaultBackground: ColorTheme.Colors.t => Revery.Color.t;
let defaultForeground: ColorTheme.Colors.t => Revery.Color.t;

type highlights = (int, list(ThemeToken.t));
let getLinesAndHighlights:
  (~colorTheme: ColorTheme.Colors.t, ~terminalId: int) =>
  (array(string), list(highlights));

// BUFFERRENDERER

[@deriving show]
type rendererState = {
  title: string,
  id: int,
  insertMode: bool,
};

let bufferRendererReducer: (rendererState, msg) => rendererState;

// COMMANDS

module Commands: {
  module New: {
    let horizontal: Command.t(msg);
    let vertical: Command.t(msg);
    let current: Command.t(msg);
  };

  module Oni: {
    let normalMode: Command.t(msg);
    let insertMode: Command.t(msg);
  };
};

module Configuration: {
  let fontFamily: Config.Schema.setting(option(string));
  let fontSize: Config.Schema.setting(option(float));
  let fontSmoothing: Config.Schema.setting(option(FontSmoothing.t));
  let fontWeight: Config.Schema.setting(option(Revery.Font.Weight.t));
  let fontLigatures: Config.Schema.setting(option(FontLigatures.t));
};

module TerminalView: {
  let make:
    (
      ~isActive: bool,
      ~config: Config.resolver,
      ~id: int,
      ~terminals: t,
      ~theme: Oni_Core.ColorTheme.Colors.t,
      ~dispatch: msg => unit,
      unit
    ) =>
    Revery.UI.element;
};

// CONTRIBUTIONS

module Contributions: {
  let colors: list(ColorTheme.Schema.definition);
  let commands: list(Command.t(msg));
  let configuration: list(Config.Schema.spec);
  let keybindings: list(Feature_Input.Schema.keybinding);

  let pane: Feature_Pane.Schema.t(t, msg);
};

module Testing: {
  let newTerminalMsg:
    (
      ~cmd: option(string),
      ~splitDirection: SplitDirection.t,
      ~closeOnExit: bool
    ) =>
    msg;
};
