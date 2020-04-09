open Oni_Core;

module ExtHostClient = Oni_Extensions.ExtHostClient;

type terminal = {
  id: int,
  cmd: string,
  rows: int,
  columns: int,
  pid: option(int),
  title: option(string),
  screen: ReveryTerminal.Screen.t,
  cursor: ReveryTerminal.Cursor.t,
};

type t;

let initial: t;

let toList: t => list(terminal);

let getTerminalOpt: (int, t) => option(terminal);

type splitDirection =
  | Vertical
  | Horizontal
  | Current;

[@deriving show({with_path: false})]
type msg =
  | NewTerminal({
      cmd: option(string),
      splitDirection,
    })
  | Resized({
      id: int,
      rows: int,
      columns: int,
    })
  | KeyPressed({
      id: int,
      key: string,
    })
  | Service(Service_Terminal.msg);

type outmsg =
  | Nothing
  | Effect(Isolinear.Effect.t(msg))
  | TerminalCreated({
      name: string,
      splitDirection,
    });

let shouldHandleInput: string => bool;

let update: (~config: Config.resolver, t, msg) => (t, outmsg);

let subscription:
  (~workspaceUri: Uri.t, ExtHostClient.t, t) => Isolinear.Sub.t(msg);

let shellCmd: string;

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

let theme: ColorTheme.resolver => ReveryTerminal.Theme.t;
let defaultBackground: ColorTheme.resolver => Revery.Color.t;
let defaultForeground: ColorTheme.resolver => Revery.Color.t;

type highlights = (int, list(ColorizedToken.t));
let getLinesAndHighlights:
  (~colorTheme: ColorTheme.resolver, ~terminalId: int) =>
  (array(string), list(highlights));

module Contributions: {let colors: list(ColorTheme.Schema.definition);};
