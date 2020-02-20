open Oni_Core;
open Utility;

type terminal = {
  id: int,
  cmd: string,
  rows: int,
  columns: int,
};

type t = {
  idToTerminal: IntMap.t(terminal),
  lastId: int,
};

let initial = {idToTerminal: IntMap.empty, lastId: 0};

let getNextId = ({lastId, _}) => lastId + 1;

let getBufferName = id => "oni-terminal://" ++ string_of_int(id);

let toList = ({idToTerminal, _}) => IntMap.bindings(idToTerminal);

type msg =
  | Started({
      id: int,
      cmd: string,
    })
  | ScreenUpdated;

let update = (extHostClient, model: t, msg) => {
  switch (msg) {
  | Started({id, cmd}) =>
    let idToTerminal =
      IntMap.add(id, {id, cmd, rows: 80, columns: 40}, model.idToTerminal);
    let lastId = max(id, model.lastId);
    ({idToTerminal, lastId}, Isolinear.Effect.none);
  | _ => (model, Isolinear.Effect.none)
  };
};
