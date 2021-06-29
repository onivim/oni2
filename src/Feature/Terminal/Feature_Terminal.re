open Oni_Core;

module Terminal = Terminal;

// MODEL
include Model;

type outmsg =
  | Nothing
  | Effect(Isolinear.Effect.t(msg))
  | NotifyError(string)
  | SwitchToNormalMode
  | ClosePane({paneId: string})
  | TogglePane({paneId: string})
  | TerminalCreated({
      name: string,
      splitDirection: SplitDirection.t,
    })
  | TerminalExit({
      terminalId: int,
      exitCode: int,
      shouldClose: bool,
    });

// CONFIGURATION

module Configuration = Configuration;

let shouldClose = (~id, {idToTerminal, _}) => {
  IntMap.find_opt(id, idToTerminal)
  |> Option.map(Terminal.closeOnExit)
  |> Option.value(~default=true);
};

let updateById = (id, f, model) => {
  let idToTerminal = IntMap.update(id, Option.map(f), model.idToTerminal);
  {...model, idToTerminal};
};

let updateAll = (f, model) => {
  let idToTerminal = IntMap.map(f, model.idToTerminal);
  {...model, idToTerminal};
};

let recomputeFont = (~config, ~font: Service_Font.font, model) => {
  let resolvedFont =
    Service_Font.resolveWithFallback(
      Revery.Font.Weight.Normal,
      font.fontFamily,
    );

  let lineHeight =
    Configuration.lineHeight.get(config)
    |> Oni_Core.Utility.OptionEx.value_or_lazy(() => {
         Feature_Configuration.GlobalConfiguration.Editor.lineHeight.get(
           config,
         )
       });

  let Service_Font.{fontSize, smoothing, _} = font;

  let lineHeightSize =
    Oni_Core.LineHeight.calculate(~measuredFontHeight=fontSize, lineHeight);
  let resolvedFont =
    EditorTerminal.Font.make(
      ~smoothing,
      ~size=fontSize,
      ~lineHeight=lineHeightSize,
      resolvedFont,
    );

  {...model, font, resolvedFont}
  |> updateAll(Terminal.setFont(~font=resolvedFont));
};

let configurationChanged = (~config, model) => {
  recomputeFont(~config, ~font=model.font, model);
};

let update =
    (
      ~clientServer: Feature_ClientServer.model,
      ~config: Config.resolver,
      model: t,
      msg,
    ) => {
  switch (msg) {
  | Terminal({id, msg}) => (
      model |> updateById(id, Terminal.update(msg)),
      Nothing,
    )

  | Font(Service_Font.FontLoaded(font)) =>
    let model' = model |> recomputeFont(~config, ~font);
    (model', Nothing);

  | Font(Service_Font.FontLoadError(message)) => (
      model,
      NotifyError(message),
    )

  | Command(ToggleIntegratedTerminal) => (
      model,
      TogglePane({paneId: "workbench.panel.terminal"}),
    )

  | StartPaneTerminal =>
    let cmdToUse =
      switch (Revery.Environment.os) {
      | Windows(_) => Configuration.Shell.windows.get(config)
      | Mac(_) => Configuration.Shell.osx.get(config)
      | Linux(_) => Configuration.Shell.linux.get(config)
      | _ => Configuration.shellCmd
      };

    let arguments =
      switch (Revery.Environment.os) {
      | Windows(_) => Configuration.ShellArgs.windows.get(config)
      | Mac(_) => Configuration.ShellArgs.osx.get(config)
      | Linux(_) => Configuration.ShellArgs.linux.get(config)
      | _ => []
      };

    let defaultEnvVariables =
      [
        ("ONIVIM_TERMINAL", BuildInfo.version),
        (
          "ONIVIM2_PARENT_PIPE",
          clientServer
          |> Feature_ClientServer.pipe
          |> Exthost.NamedPipe.toString,
        ),
      ]
      |> List.to_seq
      |> StringMap.of_seq;

    let env =
      Exthost.ShellLaunchConfig.(
        switch (Revery.Environment.os) {
        // Windows - simply inherit from the running process
        | Windows(_) => Additive(defaultEnvVariables)

        // Mac - inherit (we rely on the '-l' flag to pick up user config)
        | Mac(_) => Additive(defaultEnvVariables)

        // For Linux, there's a few stray variables that may come in from the AppImage
        // for example - LD_LIBRARY_PATH in issue #2040. We need to clear those out.
        | Linux(_) =>
          switch (
            Sys.getenv_opt("ONI2_ORIG_PATH"),
            Sys.getenv_opt("ONI2_ORIG_LD_LIBRARY_PATH"),
          ) {
          // We're running from the AppImage, which tracks the original env.
          | (Some(origPath), Some(origLdLibPath)) =>
            let envVariables =
              [("PATH", origPath), ("LD_LIBRARY_PATH", origLdLibPath)]
              |> List.to_seq
              |> StringMap.of_seq;

            let merge = (maybeA, maybeB) =>
              switch (maybeA, maybeB) {
              | (Some(_) as a, Some(_)) => a
              | (Some(_) as a, _) => a
              | (None, Some(_) as b) => b
              | (None, None) => None
              };
            let allVariables =
              StringMap.merge(
                _key => merge,
                envVariables,
                defaultEnvVariables,
              );

            Additive(allVariables);

          // All other cases - just inherit. Maybe not running from AppImage.
          | _ => Additive(defaultEnvVariables)
          }

        | _ => Additive(defaultEnvVariables)
        }
      );

    let launchConfig =
      Exthost.ShellLaunchConfig.{
        name: "Terminal",
        arguments,
        executable: cmdToUse,
        env,
      };

    let id = model.nextId;
    let idToTerminal =
      IntMap.add(
        id,
        Terminal.initial(~font=model.resolvedFont, ~id, ~launchConfig, ()),
        model.idToTerminal,
      );
    (
      {...model, idToTerminal, nextId: id + 1, paneTerminalId: Some(id)},
      Nothing,
    );
  | Command(NewTerminal({cmd, splitDirection, closeOnExit})) =>
    let cmdToUse =
      switch (cmd) {
      | None =>
        switch (Revery.Environment.os) {
        | Windows(_) => Configuration.Shell.windows.get(config)
        | Mac(_) => Configuration.Shell.osx.get(config)
        | Linux(_) => Configuration.Shell.linux.get(config)
        | _ => Configuration.shellCmd
        }
      | Some(specifiedCommand) => specifiedCommand
      };

    let arguments =
      switch (Revery.Environment.os) {
      | Windows(_) => Configuration.ShellArgs.windows.get(config)
      | Mac(_) => Configuration.ShellArgs.osx.get(config)
      | Linux(_) => Configuration.ShellArgs.linux.get(config)
      | _ => []
      };

    let defaultEnvVariables =
      [
        ("ONIVIM_TERMINAL", BuildInfo.version),
        (
          "ONIVIM2_PARENT_PIPE",
          clientServer
          |> Feature_ClientServer.pipe
          |> Exthost.NamedPipe.toString,
        ),
      ]
      |> List.to_seq
      |> StringMap.of_seq;

    let env =
      Exthost.ShellLaunchConfig.(
        switch (Revery.Environment.os) {
        // Windows - simply inherit from the running process
        | Windows(_) => Additive(defaultEnvVariables)

        // Mac - inherit (we rely on the '-l' flag to pick up user config)
        | Mac(_) => Additive(defaultEnvVariables)

        // For Linux, there's a few stray variables that may come in from the AppImage
        // for example - LD_LIBRARY_PATH in issue #2040. We need to clear those out.
        | Linux(_) =>
          switch (
            Sys.getenv_opt("ONI2_ORIG_PATH"),
            Sys.getenv_opt("ONI2_ORIG_LD_LIBRARY_PATH"),
          ) {
          // We're running from the AppImage, which tracks the original env.
          | (Some(origPath), Some(origLdLibPath)) =>
            let envVariables =
              [("PATH", origPath), ("LD_LIBRARY_PATH", origLdLibPath)]
              |> List.to_seq
              |> StringMap.of_seq;

            let merge = (maybeA, maybeB) =>
              switch (maybeA, maybeB) {
              | (Some(_) as a, Some(_)) => a
              | (Some(_) as a, _) => a
              | (None, Some(_) as b) => b
              | (None, None) => None
              };
            let allVariables =
              StringMap.merge(
                _key => merge,
                envVariables,
                defaultEnvVariables,
              );

            Additive(allVariables);

          // All other cases - just inherit. Maybe not running from AppImage.
          | _ => Additive(defaultEnvVariables)
          }

        | _ => Additive(defaultEnvVariables)
        }
      );

    let launchConfig =
      Exthost.ShellLaunchConfig.{
        name: "Terminal",
        arguments,
        executable: cmdToUse,
        env,
      };

    let id = model.nextId;
    let idToTerminal =
      IntMap.add(
        id,
        Terminal.initial(
          ~closeOnExit,
          ~font=model.resolvedFont,
          ~id,
          ~launchConfig,
          (),
        ),
        model.idToTerminal,
      );
    (
      {...model, idToTerminal, nextId: id + 1},
      TerminalCreated({name: getBufferName(id, cmdToUse), splitDirection}),
    );

  | Command(InsertMode) =>
    // Used for the renderer state
    (model, Nothing)

  | Command(NormalMode) => (model, SwitchToNormalMode)

  | PaneKeyPressed(key) =>
    let eff =
      model.paneTerminalId
      |> Option.map(id => {
           let inputEffect =
             Service_Terminal.Effect.input(~id, key)
             |> Isolinear.Effect.map(msg => Service(msg));

           Effect(inputEffect);
         })
      |> Option.value(~default=Nothing);

    let model' =
      model.paneTerminalId
      |> Option.map(paneId =>
           updateById(paneId, Terminal.scrollToBottom, model)
         )
      |> Option.value(~default=model);
    (model', eff);

  | KeyPressed({id, key}) =>
    let inputEffect =
      Service_Terminal.Effect.input(~id, key)
      |> Isolinear.Effect.map(msg => Service(msg));
    let model' = model |> updateById(id, Terminal.scrollToBottom);
    (model', Effect(inputEffect));

  | Pasted({id, text}) =>
    let inputEffect =
      Service_Terminal.Effect.paste(~id, text)
      |> Isolinear.Effect.map(msg => Service(msg));
    let model' = model |> updateById(id, Terminal.scrollToBottom);
    (model', Effect(inputEffect));

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

  | Service(ScreenUpdated({id, screen, cursor})) =>
    let newModel =
      updateById(id, Terminal.updateScreen(~screen, ~cursor), model);
    (newModel, Nothing);

  | Service(ProcessExit({id, exitCode})) =>
    if (Some(id) == model.paneTerminalId) {
      (
        {...model, paneTerminalId: None},
        ClosePane({paneId: "workbench.panel.terminal"}),
      );
    } else {
      (
        model,
        TerminalExit({
          terminalId: id,
          exitCode,
          shouldClose: shouldClose(~id, model),
        }),
      );
    }
  };
};

let subscription =
    (
      ~defaultFontFamily: string,
      ~defaultFontSize: float,
      ~defaultFontWeight: Revery.Font.Weight.t,
      ~defaultLigatures: FontLigatures.t,
      ~defaultSmoothing: FontSmoothing.t,
      ~config,
      ~setup,
      ~workspaceUri,
      extHostClient,
      model: t,
    ) => {
  let terminalFontFamily =
    Configuration.fontFamily.get(config)
    |> Option.value(~default=defaultFontFamily);

  let terminalFontSize =
    Configuration.fontSize.get(config)
    |> Option.value(~default=defaultFontSize);

  let terminalFontWeight =
    Configuration.fontWeight.get(config)
    |> Option.value(~default=defaultFontWeight);

  let terminalFontLigatures =
    Configuration.fontLigatures.get(config)
    |> Option.value(~default=defaultLigatures);

  let terminalFontSmoothing =
    Configuration.fontSmoothing.get(config)
    |> Option.value(~default=defaultSmoothing);

  let fontSubscription =
    Service_Font.Sub.font(
      ~uniqueId="Feature_Terminal.font",
      ~fontFamily=terminalFontFamily,
      ~fontSize=terminalFontSize,
      ~fontWeight=terminalFontWeight,
      ~fontSmoothing=terminalFontSmoothing,
      ~fontLigatures=terminalFontLigatures,
    )
    |> Isolinear.Sub.map(msg => Font(msg));

  let subs =
    model
    |> toList
    |> List.map((terminal: terminal) => {
         Service_Terminal.Sub.terminal(
           ~setup,
           ~id=terminal.id,
           ~launchConfig=terminal.launchConfig,
           ~rows=terminal.rows,
           ~columns=terminal.columns,
           ~workspaceUri,
           ~extHostClient,
         )
       })
    |> Isolinear.Sub.batch
    |> Isolinear.Sub.map(msg => Service(msg));

  [fontSubscription, subs] |> Isolinear.Sub.batch;
};

// COLORS
include Colors;

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

type highlights = (int, list(ThemeToken.t));

module TermScreen = EditorTerminal.Screen;

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

  // TODO: Hook up the bold/italic in revery-terminal
  let newToken =
    ThemeToken.{
      index: column,
      backgroundColor: bg,
      foregroundColor: fg,
      syntaxScope: SyntaxScope.none,
      bold: false,
      italic: false,
    };

  switch (tokens) {
  | [ThemeToken.{foregroundColor, backgroundColor, _} as ct, ...tail]
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
       module TermScreen = EditorTerminal.Screen;
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

  let toggleIntegratedTerminal =
    define(
      ~category="Terminal",
      ~title="Toggle Integrated Terminal",
      "workbench.action.terminal.toggleTerminal",
      Command(ToggleIntegratedTerminal),
    );

  let newIntegratedTerminal =
    define(
      ~category="Terminal",
      ~title="New Integrated Terminal",
      "workbench.action.terminal.new",
      // TODO: Handle multiple instances
      Command(ToggleIntegratedTerminal),
    );

  module New = {
    let horizontal =
      define(
        ~category="Terminal",
        ~title="Open terminal in new horizontal split",
        "terminal.new.horizontal",
        Command(
          NewTerminal({
            cmd: None,
            splitDirection: Horizontal,
            closeOnExit: true,
          }),
        ),
      );
    let vertical =
      define(
        ~category="Terminal",
        ~title="Open terminal in new vertical split",
        "terminal.new.vertical",
        Command(
          NewTerminal({
            cmd: None,
            splitDirection: Vertical({shouldReuse: false}),
            closeOnExit: true,
          }),
        ),
      );
    let current =
      define(
        ~category="Terminal",
        ~title="Open terminal in current window",
        "terminal.new.current",
        Command(
          NewTerminal({
            cmd: None,
            splitDirection: Current,
            closeOnExit: true,
          }),
        ),
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
      toggleIntegratedTerminal,
      newIntegratedTerminal,
      New.horizontal,
      New.vertical,
      New.current,
      Oni.normalMode,
      Oni.insertMode,
    ];

  let configuration =
    Configuration.[
      Shell.windows.spec,
      Shell.linux.spec,
      Shell.osx.spec,
      ShellArgs.windows.spec,
      ShellArgs.linux.spec,
      ShellArgs.osx.spec,
      fontFamily.spec,
      fontSize.spec,
      fontWeight.spec,
      fontLigatures.spec,
      fontSmoothing.spec,
      lineHeight.spec,
    ];

  let keybindings = {
    Feature_Input.Schema.[
      // Global
      bind(
        ~key="<C-`>",
        ~command=Commands.toggleIntegratedTerminal.id,
        ~condition=WhenExpr.Value(True),
      ),
      // Insert mode -> normal mdoe
      bind(
        ~key="<C-\\><C-N>",
        ~command=Commands.Oni.normalMode.id,
        ~condition="terminalFocus && insertMode" |> WhenExpr.parse,
      ),
      bind(
        ~key="<C-\\>n",
        ~command=Commands.Oni.normalMode.id,
        ~condition="terminalFocus && insertMode" |> WhenExpr.parse,
      ),
      // Normal mode -> insert mode
      bind(
        ~key="o",
        ~command=Commands.Oni.insertMode.id,
        ~condition="terminalFocus && normalMode" |> WhenExpr.parse,
      ),
      bind(
        ~key="<S-O>",
        ~command=Commands.Oni.insertMode.id,
        ~condition="terminalFocus && normalMode" |> WhenExpr.parse,
      ),
      bind(
        ~key="Shift+a",
        ~command=Commands.Oni.insertMode.id,
        ~condition="terminalFocus && normalMode" |> WhenExpr.parse,
      ),
      bind(
        ~key="a",
        ~command=Commands.Oni.insertMode.id,
        ~condition="terminalFocus && normalMode" |> WhenExpr.parse,
      ),
      bind(
        ~key="i",
        ~command=Commands.Oni.insertMode.id,
        ~condition="terminalFocus && normalMode" |> WhenExpr.parse,
      ),
      bind(
        ~key="Shift+i",
        ~command=Commands.Oni.insertMode.id,
        ~condition="terminalFocus && normalMode" |> WhenExpr.parse,
      ),
      // Paste - Windows:
      bind(
        ~key="<C-V>",
        ~command=Feature_Clipboard.Commands.paste.id,
        ~condition="terminalFocus && insertMode && isWin" |> WhenExpr.parse,
      ),
      // Paste - Linux:
      bind(
        ~key="<C-S-V>",
        ~command=Feature_Clipboard.Commands.paste.id,
        ~condition="terminalFocus && insertMode && isLinux" |> WhenExpr.parse,
      ),
      // Paste - Mac:
      bind(
        ~key="<D-V>",
        ~command=Feature_Clipboard.Commands.paste.id,
        ~condition="terminalFocus && insertMode && isMac" |> WhenExpr.parse,
      ),
    ];
  };

  let focusedContextKeys = {
    WhenExpr.ContextKeys.(
      [
        Schema.bool("terminalFocus", _ => true),
        Schema.bool("terminalProcessSupported", _ => true),
      ]
      |> Schema.fromList
      |> fromSchema()
    );
  };

  let pane: Feature_Pane.Schema.t(t, msg) = {
    Feature_Pane.Schema.(
      panel(
        ~title="Terminal",
        ~id=Some("workbench.panel.terminal"),
        ~buttons=
          (~font as _, ~theme as _, ~dispatch as _, ~model as _) =>
            Revery.UI.React.empty,
        ~commands=_model => [],
        ~contextKeys=
          (~isFocused, _model) =>
            if (isFocused) {focusedContextKeys} else {
              WhenExpr.ContextKeys.empty
            },
        ~sub=
          (~isFocused, model) =>
            if (isFocused && model.paneTerminalId == None) {
              SubEx.value(
                ~uniqueId="Feature_Terminal.Pane.start",
                StartPaneTerminal,
              );
            } else {
              Isolinear.Sub.none;
            },
        ~view=
          (
            ~config,
            ~editorFont as _,
            ~font as _,
            ~isFocused,
            ~iconTheme as _,
            ~languageInfo as _,
            ~workingDirectory as _,
            ~theme,
            ~dispatch,
            ~model,
          ) => {
            model.paneTerminalId
            |> Utility.OptionEx.flatMap(id => getTerminalOpt(id, model))
            |> Option.map(terminal => {
                 <TerminalView.Terminal
                   isActive=isFocused
                   config
                   terminal
                   theme
                   dispatch
                 />
               })
            |> Option.value(~default=Revery.UI.React.empty)
          },
        ~keyPressed=key => PaneKeyPressed(key),
      )
    );
  };
};

module TerminalView = TerminalView;

module Testing = {
  let newTerminalMsg = Msg.terminalCreatedFromVim;
};
