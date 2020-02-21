open Oni_Core;

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
  nextId: int,
};

let initial = {idToTerminal: IntMap.empty, nextId: 0};

let getBufferName = id => "oni://terminal/" ++ string_of_int(id);

let toList = ({idToTerminal, _}) => IntMap.bindings(idToTerminal);

let getTerminalOpt = (id, {idToTerminal, _}) =>
  IntMap.find_opt(id, idToTerminal);

type splitDirection =
  | Vertical
  | Horizontal;

type outmsg =
  | Nothing
  | OpenBuffer({
      name: string,
      splitDirection,
    });

type msg =
  | Started({
      cmd: string,
      splitDirection,
    })
  | Service(Service_Terminal.msg);

let update = (_extHostClient, model: t, msg) => {
  switch (msg) {
  | Started({cmd, splitDirection}) =>
    let id = model.nextId;
    let idToTerminal =
      IntMap.add(
        id,
        {id, cmd, rows: 80, columns: 40, pid: None, title: None},
        model.idToTerminal,
      );
    (
      {idToTerminal, nextId: id + 1},
      OpenBuffer({name: getBufferName(id), splitDirection}),
    );
  | Service(ProcessStarted({id, pid})) =>
    let idToTerminal =
      IntMap.update(
        id,
        Option.map(term => {...term, pid: Some(pid)}),
        model.idToTerminal,
      );
    ({...model, idToTerminal}, Nothing);
  | Service(ProcessTitleSet({id, title})) =>
    let idToTerminal =
      IntMap.update(
        id,
        Option.map(term => {...term, title: Some(title)}),
        model.idToTerminal,
      );
    ({...model, idToTerminal}, Nothing);
  | _ => (model, Nothing)
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
