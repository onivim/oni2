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
  paneTerminalId: option(int),
  font: Service_Font.font,
};

let font = ({font, _}) => font;
let initial = {
  idToTerminal: IntMap.empty,
  nextId: 0,
  paneTerminalId: None,
  font: Service_Font.default(),
};

let getBufferName = (id, cmd) =>
  Printf.sprintf("oni://terminal/%d/%s", id, cmd);

let toList = ({idToTerminal, _}) =>
  idToTerminal |> IntMap.bindings |> List.map(snd);

let getTerminalOpt = (id, {idToTerminal, _}) =>
  IntMap.find_opt(id, idToTerminal);

// UPDATE

[@deriving show({with_path: false})]
type command =
  | NewTerminal({
      cmd: option(string),
      splitDirection: SplitDirection.t,
      closeOnExit: bool,
    })
  | ToggleIntegratedTerminal
  | NormalMode
  | InsertMode;

[@deriving show({with_path: false})]
type msg =
  | Command(command)
  | Font(Service_Font.msg)
  | Resized({
      id: int,
      rows: int,
      columns: int,
    })
  | KeyPressed({
      id: int,
      key: string,
    })
  | PaneKeyPressed(string)
  | StartPaneTerminal
  | Pasted({
      id: int,
      text: string,
    })
  | Service(Service_Terminal.msg);

module Msg = {
  let terminalCreatedFromVim = (~cmd, ~splitDirection, ~closeOnExit) =>
    Command(NewTerminal({cmd, splitDirection, closeOnExit}));

  let keyPressed = (~id, key) => KeyPressed({id, key});

  let pasted = (~id, text) => Pasted({id, text});
};
