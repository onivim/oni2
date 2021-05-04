open Oni_Core;

// MODEL

type terminal = {
  id: int,
  launchConfig: Exthost.ShellLaunchConfig.t,
  rows: int,
  columns: int,
  pid: option(int),
  title: option(string),
  screen: ReveryTerminal.Screen.t,
  cursor: ReveryTerminal.Cursor.t,
  closeOnExit: bool,
};

type t = {
  idToTerminal: IntMap.t(terminal),
  nextId: int,
};

let initial = {idToTerminal: IntMap.empty, nextId: 0};

let getBufferName = (id, cmd) =>
  Printf.sprintf("oni://terminal/%d/%s", id, cmd);

let toList = ({idToTerminal, _}) =>
  idToTerminal |> IntMap.bindings |> List.map(snd);

let getTerminalOpt = (id, {idToTerminal, _}) =>
  IntMap.find_opt(id, idToTerminal);

// UPDATE

[@deriving show({with_path: false})]
type splitDirection =
  | Vertical
  | Horizontal
  | Current;

[@deriving show({with_path: false})]
type command =
  | NewTerminal({
      cmd: option(string),
      splitDirection,
      closeOnExit: bool,
    })
  | NormalMode
  | InsertMode;

[@deriving show({with_path: false})]
type msg =
  | Command(command)
  | Pane(Pane.msg)
  | Resized({
      id: int,
      rows: int,
      columns: int,
    })
  | KeyPressed({
      id: int,
      key: string,
    })
  | Pasted({
      id: int,
      text: string,
    })
  | Service(Service_Terminal.msg);
