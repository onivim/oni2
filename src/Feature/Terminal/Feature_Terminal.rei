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
  | Horizontal;

[@deriving show({with_path: false})]
type msg =
  | NewTerminal({splitDirection})
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

let update: (ExtHostClient.t, t, msg) => (t, outmsg);

let subscription:
  (~workspaceUri: Uri.t, ExtHostClient.t, t) => Isolinear.Sub.t(msg);

let shellCmd: string;
