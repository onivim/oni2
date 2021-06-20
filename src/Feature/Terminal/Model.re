open Oni_Core;

// MODEL

type terminal = Terminal.t;

type t = {
  idToTerminal: IntMap.t(terminal),
  nextId: int,
  paneTerminalId: option(int),
  font: Service_Font.font,
  resolvedFont: EditorTerminal.Font.t,
};

let font = ({font, _}) => font;
let initial = {
  idToTerminal: IntMap.empty,
  nextId: 0,
  paneTerminalId: None,
  font: Service_Font.default(),
  resolvedFont:
    EditorTerminal.Font.make(
      ~size=14.0,
      ~lineHeight=14.0,
      Service_Font.resolveWithFallback(
        Revery.Font.Weight.Normal,
        Service_Font.default().fontFamily,
      ),
    ),
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
  | Terminal({
      id: int,
      msg: Terminal.msg,
    })
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
