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

let shellCmd = if (Sys.win32) {"cmd.exe"} else {"/bin/bash"};

let inputToIgnore = ["<C-w>", "<C-h>", "<C-j>", "<C-k>", "<C-l>"];

let shouldHandleInput = str => {
  !
    List.exists(s => str == s, inputToIgnore);
    // pick what keys should be ignored by the terminal.
    // One option would be a configuration setting that lets us
    // better, more customizable way to manage this, though.
    // the user can get out of the terminal. We should have a
    // HACK: Let the window motion keys pass through, so that
};

let updateById = (id, f, model) => {
  let idToTerminal = IntMap.update(id, Option.map(f), model.idToTerminal);
  {...model, idToTerminal};
};

let update = (model: t, msg) => {
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
          rows: 40,
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
  | KeyPressed({id, key}) =>
    let inputEffect =
      Service_Terminal.Effect.input(~id, key)
      |> Isolinear.Effect.map(msg => Service(msg));

    (model, Effect(inputEffect));
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

module Colors = {
  let background = "terminal.background";
  let foreground = "terminal.foreground";
  let ansiBlack = "terminal.ansiBlack";
  let ansiRed = "terminal.ansiRed";
  let ansiGreen = "terminal.ansiGreen";
  let ansiYellow = "terminal.ansiYellow";
  let ansiBlue = "terminal.ansiBlue";
  let ansiMagenta = "terminal.ansiMagenta";
  let ansiCyan = "terminal.ansiCyan";
  let ansiWhite = "terminal.ansiWhite";
  let ansiBrightBlack = "terminal.ansiBrightBlack";
  let ansiBrightRed = "terminal.ansiBrightRed";
  let ansiBrightGreen = "terminal.ansiBrightGreen";
  let ansiBrightYellow = "terminal.ansiBrightYellow";
  let ansiBrightBlue = "terminal.ansiBrightBlue";
  let ansiBrightMagenta = "terminal.ansiBrightMagenta";
  let ansiBrightCyan = "terminal.ansiBrightCyan";
  let ansiBrightWhite = "terminal.ansiBrightWhite";
};

// CONTRIBUTIONS

module Contributions = {
  open Revery;

  let colors =
    ColorTheme.Defaults.[
      ("terminal.background", color(Color.rgb_int(0, 0, 0)) |> uniform),
      (
        "terminal.foreground",
        color(Color.rgb_int(233, 235, 235)) |> uniform,
      ),
      ("terminal.ansiBlack", color(Color.rgb_int(0, 0, 0)) |> uniform),
      ("terminal.ansiRed", color(Color.rgb_int(194, 54, 33)) |> uniform),
      ("terminal.ansiGreen", color(Color.rgb_int(37, 188, 36)) |> uniform),
      (
        "terminal.ansiYellow",
        color(Color.rgb_int(173, 173, 39)) |> uniform,
      ),
      ("terminal.ansiBlue", color(Color.rgb_int(73, 46, 225)) |> uniform),
      (
        "terminal.ansiMagenta",
        color(Color.rgb_int(211, 56, 211)) |> uniform,
      ),
      ("terminal.ansiCyan", color(Color.rgb_int(51, 197, 200)) |> uniform),
      (
        "terminal.ansiWhite",
        color(Color.rgb_int(203, 204, 205)) |> uniform,
      ),
      (
        "terminal.ansiBrightBlack",
        color(Color.rgb_int(129, 131, 131)) |> uniform,
      ),
      (
        "terminal.ansiBrightRed",
        color(Color.rgb_int(252, 57, 31)) |> uniform,
      ),
      (
        "terminal.ansiBrightGreen",
        color(Color.rgb_int(49, 231, 34)) |> uniform,
      ),
      (
        "terminal.ansiBrightYellow",
        color(Color.rgb_int(234, 236, 35)) |> uniform,
      ),
      (
        "terminal.ansiBrightBlue",
        color(Color.rgb_int(88, 51, 255)) |> uniform,
      ),
      (
        "terminal.ansiBrightCyan",
        color(Color.rgb_int(20, 240, 240)) |> uniform,
      ),
      (
        "terminal.ansiBrightMagenta",
        color(Color.rgb_int(20, 240, 240)) |> uniform,
      ),
      (
        "terminal.ansiBrightWhite",
        color(Color.rgb_int(233, 235, 235)) |> uniform,
      ),
    ]
    |> ColorTheme.Defaults.fromList;
};
