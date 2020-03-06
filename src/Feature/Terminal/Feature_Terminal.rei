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

let update: (t, msg) => (t, outmsg);

let subscription:
  (~workspaceUri: Uri.t, ExtHostClient.t, t) => Isolinear.Sub.t(msg);

let shellCmd: string;

module Colors: {
  let background: string;
  let foreground: string;
  let ansiBlack: string;
  let ansiRed: string;
  let ansiGreen: string;
  let ansiYellow: string;
  let ansiBlue: string;
  let ansiMagenta: string;
  let ansiCyan: string;
  let ansiWhite: string;
  let ansiBrightBlack: string;
  let ansiBrightRed: string;
  let ansiBrightGreen: string;
  let ansiBrightYellow: string;
  let ansiBrightBlue: string;
  let ansiBrightMagenta: string;
  let ansiBrightCyan: string;
  let ansiBrightWhite: string;
};

module Contributions: {let colors: ColorTheme.Defaults.t;};
