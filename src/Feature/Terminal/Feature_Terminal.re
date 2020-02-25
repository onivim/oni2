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

[@deriving show({with_path: false})]
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
  | Service(Service_Terminal.msg);

type outmsg =
  | Nothing
  | TerminalCreated({
      name: string,
      splitDirection,
    });

let shellCmd = if (Sys.win32) {"cmd.exe"} else {"/bin/bash"};

let updateById = (id, f, model) => {
  let idToTerminal = IntMap.update(id, Option.map(f), model.idToTerminal);
  {...model, idToTerminal};
};

let update = (_extHostClient, model: t, msg) => {
  switch (msg) {
  | NewTerminal({splitDirection}) =>
    let cmd = shellCmd;
    let id = model.nextId;
    let idToTerminal =
      IntMap.add(
        id,
        {
          id,
          cmd,
          rows: 80,
          columns: 40,
          pid: None,
          title: None,
          screen: ReveryTerminal.Screen.initial,
          cursor: ReveryTerminal.Cursor.{row: 0, column: 0, visible: false},
        },
        model.idToTerminal,
      );
    (
      {idToTerminal, nextId: id + 1},
      TerminalCreated({name: getBufferName(id), splitDirection}),
    );
  | Resized({id, rows, columns}) =>
    let newModel = updateById(id, term => {...term, rows, columns}, model);
    (newModel, Nothing);
  | Service(ProcessStarted({id, pid})) =>
    let newModel = updateById(id, term => {...term, pid: Some(pid)}, model);
    (newModel, Nothing);
  | Service(ProcessTitleChanged({id, title})) =>
    let newModel =
      updateById(id, term => {...term, title: Some(title)}, model);
    (newModel, Nothing);
  | Service(ScreenUpdated({id, screen})) =>
    let newModel = updateById(id, term => {...term, screen}, model);
    (newModel, Nothing);
  | Service(CursorMoved({id, cursor})) =>
    let newModel = updateById(id, term => {...term, cursor}, model);
    (newModel, Nothing);
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
