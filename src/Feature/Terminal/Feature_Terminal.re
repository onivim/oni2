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

let toList = ({idToTerminal, _}) =>
  idToTerminal |> IntMap.bindings |> List.map(snd);

let getTerminalOpt = (id, {idToTerminal, _}) =>
  IntMap.find_opt(id, idToTerminal);

type splitDirection =
  | Vertical
  | Horizontal;

type msg =
  | NewTerminal({splitDirection})
  | Service(Service_Terminal.msg);

type outmsg =
  | Nothing
  | TerminalCreated({
      name: string,
      splitDirection,
    });

let shellCmd = if (Sys.win32) {"cmd.exe"} else {"/bin/bash"};

let update = (_extHostClient, model: t, msg) => {
  switch (msg) {
  | NewTerminal({splitDirection}) =>
    let cmd = shellCmd;
    let id = model.nextId;
    let idToTerminal =
      IntMap.add(
        id,
        {id, cmd, rows: 80, columns: 40, pid: None, title: None},
        model.idToTerminal,
      );
    (
      {idToTerminal, nextId: id + 1},
      TerminalCreated({name: getBufferName(id), splitDirection}),
    );
  | Service(ProcessStarted({id, pid})) =>
    let idToTerminal =
      IntMap.update(
        id,
        Option.map(term => {...term, pid: Some(pid)}),
        model.idToTerminal,
      );
    ({...model, idToTerminal}, Nothing);
  | Service(ProcessTitleChanged({id, title})) =>
    let idToTerminal =
      IntMap.update(
        id,
        Option.map(term => {...term, title: Some(title)}),
        model.idToTerminal,
      );
    ({...model, idToTerminal}, Nothing);
  };
};

let subscription = (~workspaceUri, extHostClient, model: t) => {
  model
  |> toList
  |> List.map((terminal: terminal) => {
       Service_Terminal.Sub.terminal(
         ~id=terminal.id,
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
