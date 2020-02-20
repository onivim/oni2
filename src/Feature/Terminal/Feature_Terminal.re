open Oni_Core;
open Utility;

module ExtHostClient = Oni_Extensions.ExtHostClient;

type terminal = {
  id: int,
  cmd: string,
  rows: int,
  columns: int,
  pid: option(int),
  title: option(string),
};

type t = {
  idToTerminal: IntMap.t(terminal),
  lastId: int,
};

let initial = {idToTerminal: IntMap.empty, lastId: 0};

let getNextId = ({lastId, _}) => lastId + 1;

let getBufferName = id => "oni://terminal/" ++ string_of_int(id);

let toList = ({idToTerminal, _}) => IntMap.bindings(idToTerminal);

type msg =
  | Started({
      id: int,
      cmd: string,
    })
  | Service(Service_Terminal.Msg.t);

let update = (extHostClient, model: t, msg) => {
  switch (msg) {
  | Started({id, cmd}) =>
    let idToTerminal =
      IntMap.add(
        id,
        {id, cmd, rows: 80, columns: 40, pid: None, title: None},
        model.idToTerminal,
      );
    let lastId = max(id, model.lastId);
    ({idToTerminal, lastId}, Isolinear.Effect.none);
  | Service(ProcessStarted({id, pid})) =>
    let idToTerminal =
      IntMap.update(
        id,
        Option.map(term => {...term, pid: Some(pid)}),
        model.idToTerminal,
      );
    ({...model, idToTerminal}, Isolinear.Effect.none);
  | Service(ProcessTitleSet({id, title})) =>
    let idToTerminal =
      IntMap.update(
        id,
        Option.map(term => {...term, title: Some(title)}),
        model.idToTerminal,
      );
    ({...model, idToTerminal}, Isolinear.Effect.none);
  | _ => (model, Isolinear.Effect.none)
  };
};

let subscription = (~workspaceUri, extHostClient, model: t) => {
  model
  |> toList
  |> List.map(((id, terminal: terminal)) => {
       Service_Terminal.Sub.terminal(
         ~id,
         ~cmd=terminal.cmd,
         ~rows=terminal.rows,
         ~columns=terminal.columns,
         ~workspaceUri,
         ~extHostClient,
       )
     })
  |> Isolinear.Sub.batch
  |> Isolinear.Sub.map(msg => Service(msg));
};

let shellCmd = if (Sys.win32) {"cmd.exe"} else {"/bin/bash"};
