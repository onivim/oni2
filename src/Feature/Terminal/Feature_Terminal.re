open Oni_Core;
module ExtHostClient = Oni_Extensions.ExtHostClient;

// MODEL

type terminal = {
  id: int,
  cmd: string,
  arguments: list(string),
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

// UPDATE

[@deriving show({with_path: false})]
type splitDirection =
  | Vertical
  | Horizontal
  | Current;

[@deriving show({with_path: false})]
type command =
  | NewTerminal({
      cmd: option(string),
      splitDirection,
    })
  | NormalMode
  | InsertMode;

[@deriving show({with_path: false})]
type msg =
  | Command(command)
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

// CONFIGURATION

module Configuration = {
  open Oni_Core;
  open Config.Schema;

  let shellCommandWindows =
    setting("terminal.integrated.shell.windows", string, ~default=shellCmd);
  let shellCommandLinux =
    setting("terminal.integrated.shell.linux", string, ~default=shellCmd);
  let shellCommandOSX =
    setting("terminal.integrated.shell.osx", string, ~default=shellCmd);

  let shellArgsWindows =
    setting(
      "terminal.integrated.shellArgs.windows",
      list(string),
      ~default=[],
    );
  let shellArgsLinux =
    setting(
      "terminal.integrated.shellArgs.linux",
      list(string),
      ~default=[],
    );
  let shellArgsOSX =
    setting("terminal.integrated.shellArgs.osx", list(string), ~default=[]);
};

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

let update = (~config: Config.resolver, model: t, msg) => {
  switch (msg) {
  | Command(NewTerminal({cmd, splitDirection})) =>
    let cmdToUse =
      switch (cmd) {
      | None =>
        switch (Revery.Environment.os) {
        | Windows => Configuration.shellCommandWindows.get(config)
        | Mac => Configuration.shellCommandOSX.get(config)
        | Linux => Configuration.shellCommandLinux.get(config)
        | _ => shellCmd
        }
      | Some(specifiedCommand) => specifiedCommand
      };

    let arguments =
      switch (Revery.Environment.os) {
      | Windows => Configuration.shellArgsWindows.get(config)
      | Mac => Configuration.shellArgsOSX.get(config)
      | Linux => Configuration.shellArgsLinux.get(config)
      | _ => []
      };

    let id = model.nextId;
    let idToTerminal =
      IntMap.add(
        id,
        {
          id,
          arguments,
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

  | Command(InsertMode | NormalMode) =>
    // Used for the renderer state
    (model, Nothing)

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
         ~arguments=terminal.arguments,
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

// COLORS

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

let getFirstNonEmptyLine =
    (~start: int, ~direction: int, lines: array(string)) => {
  let len = Array.length(lines);

  let rec loop = idx =>
    if (idx == len || idx < 0) {
      idx;
    } else if (String.length(lines[idx]) == 0) {
      loop(idx + direction);
    } else {
      idx;
    };

  loop(start);
};

let getFirstNonEmptyLineFromTop = (lines: array(string)) => {
  getFirstNonEmptyLine(~start=0, ~direction=1, lines);
};

let getFirstNonEmptyLineFromBottom = (lines: array(string)) => {
  getFirstNonEmptyLine(~start=Array.length(lines) - 1, ~direction=-1, lines);
};

type highlights = (int, list(ColorizedToken.t));

module TermScreen = ReveryTerminal.Screen;

let addHighlightForCell =
    (~defaultBackground, ~defaultForeground, ~theme, ~cell, ~column, tokens) => {
  let fg =
    TermScreen.getForegroundColor(
      ~defaultBackground,
      ~defaultForeground,
      ~theme,
      cell,
    );
  let bg =
    TermScreen.getBackgroundColor(
      ~defaultBackground,
      ~defaultForeground,
      ~theme,
      cell,
    );

  let newToken =
    ColorizedToken.{
      index: column,
      backgroundColor: bg,
      foregroundColor: fg,
      syntaxScope: SyntaxScope.none,
    };

  switch (tokens) {
  | [ColorizedToken.{foregroundColor, backgroundColor, _} as ct, ...tail]
      when foregroundColor != fg && backgroundColor != bg => [
      newToken,
      ct,
      ...tail,
    ]
  | [] => [newToken]
  | list => list
  };
};

let getLinesAndHighlights = (~colorTheme, ~terminalId) => {
  terminalId
  |> Service_Terminal.getScreen
  |> Option.map(screen => {
       module TermScreen = ReveryTerminal.Screen;
       let totalRows = TermScreen.getTotalRows(screen);
       let columns = TermScreen.getColumns(screen);

       let lines = Array.make(totalRows, "");

       let highlights = ref([]);
       let theme = theme(colorTheme);
       let defaultBackground = defaultBackground(colorTheme);
       let defaultForeground = defaultForeground(colorTheme);

       for (lineIndex in 0 to totalRows - 1) {
         let buffer = Stdlib.Buffer.create(columns * 2);
         let lineHighlights = ref([]);
         for (column in 0 to columns - 1) {
           let cell = TermScreen.getCell(~row=lineIndex, ~column, screen);
           let codeInt = Uchar.to_int(cell.char);
           if (codeInt != 0 && codeInt <= 0x10FFFF) {
             Stdlib.Buffer.add_utf_8_uchar(buffer, cell.char);

             lineHighlights :=
               addHighlightForCell(
                 ~defaultBackground,
                 ~defaultForeground,
                 ~theme,
                 ~cell,
                 ~column,
                 lineHighlights^,
               );
           } else {
             Stdlib.Buffer.add_string(buffer, " ");
           };
         };

         let str =
           Stdlib.Buffer.contents(buffer) |> Utility.StringEx.trimRight;
         highlights :=
           [(lineIndex, lineHighlights^ |> List.rev), ...highlights^];
         lines[lineIndex] = str;
       };

       let startLine = getFirstNonEmptyLineFromTop(lines);
       let bottomLine = getFirstNonEmptyLineFromBottom(lines);

       let lines =
         Utility.ArrayEx.slice(
           ~start=startLine,
           ~length=bottomLine - startLine + 1,
           ~lines,
           (),
         );

       let highlights =
         highlights^
         |> List.map(((idx, tokens)) => (idx - startLine, tokens));

       (lines, highlights);
     })
  |> Option.value(~default=([||], []));
};

// BUFFERRENDERER

// TODO: Unify the model and renderer state. This shouldn't be needed.
[@deriving show({with_path: false})]
type rendererState = {
  title: string,
  id: int,
  insertMode: bool,
};

let bufferRendererReducer = (state, action) => {
  switch (action) {
  | Service(ProcessTitleChanged({id, title, _})) when state.id == id => {
      ...state,
      id,
      title,
    }
  | Command(NormalMode) => {...state, insertMode: false}
  | Command(InsertMode) => {...state, insertMode: true}

  | _ => state
  };
};

// COMMANDS

module Commands = {
  open Feature_Commands.Schema;

  module New = {
    let horizontal =
      define(
        ~category="Terminal",
        ~title="Open terminal in new horizontal split",
        "terminal.new.horizontal",
        Command(NewTerminal({cmd: None, splitDirection: Horizontal})),
      );
    let vertical =
      define(
        ~category="Terminal",
        ~title="Open terminal in new vertical split",
        "terminal.new.vertical",
        Command(NewTerminal({cmd: None, splitDirection: Vertical})),
      );
    let current =
      define(
        ~category="Terminal",
        ~title="Open terminal in current window",
        "terminal.new.current",
        Command(NewTerminal({cmd: None, splitDirection: Current})),
      );
  };

  module Oni = {
    let normalMode = define("oni.terminal.normalMode", Command(NormalMode));
    let insertMode = define("oni.terminal.insertMode", Command(InsertMode));
  };
};

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

  let commands =
    Commands.[
      New.horizontal,
      New.vertical,
      New.current,
      Oni.normalMode,
      Oni.insertMode,
    ];

  let configuration =
    Configuration.[
      shellCommandWindows.spec,
      shellCommandLinux.spec,
      shellCommandOSX.spec,
      shellArgsWindows.spec,
      shellArgsLinux.spec,
      shellArgsOSX.spec,
    ];
};
