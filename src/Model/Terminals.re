open Oni_Core;

type msg =
  | Started({
      id: int,
      cmd: string,
    })
  | ScreenUpdated;

type t = {
  idToTerminal: IntMap.t(Terminal.t),
  lastId: int,
};

let initial = {idToTerminal: IntMap.empty, lastId: 0};

let getNextId = ({lastId, _}) => lastId + 1;

let create = (~id, ~cmd, ~rows, ~columns, {idToTerminal, lastId}) => {
  let idToTerminal =
    IntMap.add(id, Terminal.{id, cmd, rows, columns}, idToTerminal);

  let lastId = lastId + 1;

  {lastId, idToTerminal};
};

let getBufferName = id => "oni-terminal://" ++ string_of_int(id);

let toList = ({idToTerminal, _}) => IntMap.bindings(idToTerminal);
