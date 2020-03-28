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

let getBufferName = (id, cmd) =>
  Printf.sprintf("oni://terminal/%d/%s", id, cmd);

let toList = ({idToTerminal, _}) =>
  idToTerminal |> IntMap.bindings |> List.map(snd);

let getTerminalOpt = (id, {idToTerminal, _}) =>
  IntMap.find_opt(id, idToTerminal);

[@deriving show({with_path: false})]
type splitDirection =
  | Vertical
  | Horizontal
  | Current;

[@deriving show({with_path: false})]
type msg =
  | NewTerminal({
      cmd: option(string),
      splitDirection,
    })
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

let shellCmd = ShellUtility.getDefaultShell();

// TODO: REMOVE C-T! Debugging only while waiting for key sequences...
let inputToIgnore = ["<C-w>", "<C-h>", "<C-j>", "<C-k>", "<C-l>", "<C-t>"];

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
  | NewTerminal({cmd, splitDirection}) =>
    let cmdToUse =
      switch (cmd) {
      | None => shellCmd
      | Some(specifiedCommand) => specifiedCommand
      };

    let id = model.nextId;
    let idToTerminal =
      IntMap.add(
        id,
        {
          id,
          cmd: cmdToUse,
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
      TerminalCreated({name: getBufferName(id, cmdToUse), splitDirection}),
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
  open Revery;
  open ColorTheme.Schema;

  let background =
    define("terminal.background", color(Color.rgb_int(0, 0, 0)) |> all);
  let foreground =
    define(
      "terminal.foreground",
      color(Color.rgb_int(233, 235, 235)) |> all,
    );
  let ansiBlack =
    define("terminal.ansiBlack", color(Color.rgb_int(0, 0, 0)) |> all);
  let ansiRed =
    define("terminal.ansiRed", color(Color.rgb_int(194, 54, 33)) |> all);
  let ansiGreen =
    define("terminal.ansiGreen", color(Color.rgb_int(37, 188, 36)) |> all);
  let ansiYellow =
    define(
      "terminal.ansiYellow",
      color(Color.rgb_int(173, 173, 39)) |> all,
    );
  let ansiBlue =
    define("terminal.ansiBlue", color(Color.rgb_int(73, 46, 225)) |> all);
  let ansiMagenta =
    define(
      "terminal.ansiMagenta",
      color(Color.rgb_int(211, 56, 211)) |> all,
    );
  let ansiCyan =
    define("terminal.ansiCyan", color(Color.rgb_int(51, 197, 200)) |> all);
  let ansiWhite =
    define(
      "terminal.ansiWhite",
      color(Color.rgb_int(203, 204, 205)) |> all,
    );
  let ansiBrightBlack =
    define(
      "terminal.ansiBrightBlack",
      color(Color.rgb_int(129, 131, 131)) |> all,
    );
  let ansiBrightRed =
    define(
      "terminal.ansiBrightRed",
      color(Color.rgb_int(252, 57, 31)) |> all,
    );
  let ansiBrightGreen =
    define(
      "terminal.ansiBrightGreen",
      color(Color.rgb_int(49, 231, 34)) |> all,
    );
  let ansiBrightYellow =
    define(
      "terminal.ansiBrightYellow",
      color(Color.rgb_int(234, 236, 35)) |> all,
    );
  let ansiBrightBlue =
    define(
      "terminal.ansiBrightBlue",
      color(Color.rgb_int(88, 51, 255)) |> all,
    );
  let ansiBrightMagenta =
    define(
      "terminal.ansiBrightMagenta",
      color(Color.rgb_int(20, 240, 240)) |> all,
    );
  let ansiBrightCyan =
    define(
      "terminal.ansiBrightCyan",
      color(Color.rgb_int(20, 240, 240)) |> all,
    );
  let ansiBrightWhite =
    define(
      "terminal.ansiBrightWhite",
      color(Color.rgb_int(233, 235, 235)) |> all,
    );
};

let theme = theme =>
  fun
  | 0 => Colors.ansiBlack.from(theme)
  | 1 => Colors.ansiRed.from(theme)
  | 2 => Colors.ansiGreen.from(theme)
  | 3 => Colors.ansiYellow.from(theme)
  | 4 => Colors.ansiBlue.from(theme)
  | 5 => Colors.ansiMagenta.from(theme)
  | 6 => Colors.ansiCyan.from(theme)
  | 7 => Colors.ansiWhite.from(theme)
  | 8 => Colors.ansiBrightBlack.from(theme)
  | 9 => Colors.ansiBrightRed.from(theme)
  | 10 => Colors.ansiBrightGreen.from(theme)
  | 11 => Colors.ansiBrightYellow.from(theme)
  | 12 => Colors.ansiBrightBlue.from(theme)
  | 13 => Colors.ansiBrightMagenta.from(theme)
  | 14 => Colors.ansiBrightCyan.from(theme)
  | 15 => Colors.ansiBrightWhite.from(theme)
  // For 256 colors, fall back to defaults
  | idx => ReveryTerminal.Theme.default(idx);

let defaultBackground = theme => Colors.background.from(theme);
let defaultForeground = theme => Colors.foreground.from(theme);

// CONTRIBUTIONS

module Contributions = {
  let colors =
    Colors.[
      background,
      foreground,
      ansiBlack,
      ansiRed,
      ansiGreen,
      ansiYellow,
      ansiBlue,
      ansiMagenta,
      ansiCyan,
      ansiWhite,
      ansiBrightBlack,
      ansiBrightRed,
      ansiBrightGreen,
      ansiBrightYellow,
      ansiBrightBlue,
      ansiBrightCyan,
      ansiBrightMagenta,
      ansiBrightWhite,
    ];
};

type highlight = (int, list(ColorizedToken.t));

let getLines = (~terminalId) => {
  terminalId
  |> Service_Terminal.getScreenOpt
  |> Option.map(screen => {
       module TermScreen = ReveryTerminal.Screen;
       let totalRows = TermScreen.getTotalRows(screen);
       let columns = TermScreen.getColumns(screen);

       let lines = Array.make(totalRows, "");

       for (lineIndex in 0 to totalRows - 1) {
         let buffer = Stdlib.Buffer.create(columns * 2);

         for (column in 0 to columns - 1) {
           let cell = TermScreen.getCell(lineIndex, column, screen);
           let codeInt = Uchar.to_int(cell.char);
           if (codeInt != 0 && codeInt <= 0x10FFFF) {
             Stdlib.Buffer.add_utf_8_uchar(buffer, cell.char);
           } else {
             Stdlib.Buffer.add_string(buffer, " ");
           };
         };

         let str =
           Stdlib.Buffer.contents(buffer) |> Utility.StringEx.trimRight;
         lines[lineIndex] = str;
       };
       lines;
     })
  |> Option.value(~default=[||]);
};
