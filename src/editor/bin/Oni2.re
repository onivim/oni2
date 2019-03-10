/*
 * Oni2.re
 *
 * This is the entry point for launching the editor.
 */

open Revery;
open Revery.UI;

open Rench;

open Oni_UI;
open Oni_Neovim;

module Core = Oni_Core;

/**
   This allows a stack trace to be printed when exceptions occur
 */
switch (Sys.getenv_opt("REVERY_DEBUG")) {
| Some(_) => Printexc.record_backtrace(true) |> ignore
| None => ()
};

let state = Core.State.create();
/* The 'main' function for our app */
let init = app => {
  let w =
    App.createWindow(
      ~createOptions={
        ...Window.defaultCreateOptions,
        maximized: false,
        icon: Some("logo.png"),
      },
      app,
      "Oni2",
    );

  let initVimPath = Revery.Environment.getExecutingDirectory() ++ "init.vim";
  Core.Log.debug("initVimPath: " ++ initVimPath);

  let setup = Oni_Core.Setup.init();

  let nvim =
    NeovimProcess.start(
      ~neovimPath=setup.neovimPath,
      ~args=[|"-u", initVimPath, "--noplugin", "--embed"|],
    );
  let msgpackTransport =
    MsgpackTransport.make(
      ~onData=nvim.stdout.onData,
      ~write=nvim.stdin.write,
      (),
    );

  let nvimApi = NeovimApi.make(msgpackTransport);
  let neovimProtocol = NeovimProtocol.make(nvimApi);

  let defaultThemePath =
    setup.bundledExtensionsPath ++ "/onedark-pro/themes/OneDark-Pro.json";
  let reasonSyntaxPath =
    setup.bundledExtensionsPath ++ "/vscode-reasonml/syntaxes/reason.json";

  let onScopeLoaded = s => prerr_endline("Scope loaded: " ++ s);
  let onColorMap = cm =>
    App.dispatch(app, Core.Actions.SyntaxHighlightColorMap(cm));

  let onTokens = tr =>
    App.dispatch(app, Core.Actions.SyntaxHighlightTokens(tr));

  let tmClient =
    Oni_Core.TextmateClient.start(
      ~onScopeLoaded,
      ~onColorMap,
      ~onTokens,
      setup,
      [{scopeName: "source.reason", path: reasonSyntaxPath}],
    );

  Oni_Core.TextmateClient.setTheme(tmClient, defaultThemePath);

  let render = () => {
    let state: Core.State.t = App.getState(app);
    GlobalContext.set({
      notifySizeChanged: (~width, ~height, ()) =>
        App.dispatch(
          app,
          Core.Actions.SetEditorSize(
            Core.Types.EditorSize.create(
              ~pixelWidth=width,
              ~pixelHeight=height,
              (),
            ),
          ),
        ),
      editorScroll: (~deltaY, ()) =>
        App.dispatch(app, Core.Actions.EditorScroll(deltaY)),
      openFile: neovimProtocol.openFile,
      closeFile: neovimProtocol.closeFile,
    });
    /* prerr_endline( */
    /*   "[DEBUG - STATE] Mode: " */
    /*   ++ Core.Types.Mode.show(state.mode) */
    /*   ++ " editor font measured width: " */
    /*   ++ string_of_int(state.editorFont.measuredWidth) */
    /*   ++ " editor font measured height: " */
    /*   ++ string_of_int(state.editorFont.measuredHeight), */
    /* ); */
    <Root state />;
  };

  UI.start(w, render);

  neovimProtocol.uiAttach();

  let setFont = (fontFamily, fontSize) =>
    Fontkit.fk_new_face(
      Revery.Environment.getExecutingDirectory() ++ fontFamily,
      fontSize,
      font => {
        open Oni_Core.Actions;
        open Oni_Core.Types;

        /* Measure text */
        let shapedText = Fontkit.fk_shape(font, "H");
        let firstShape = shapedText[0];
        let glyph = Fontkit.renderGlyph(font, firstShape.glyphId);

        let metrics = Fontkit.fk_get_metrics(font);
        let actualHeight =
          int_of_float(
            float_of_int(fontSize)
            *. float_of_int(metrics.height)
            /. float_of_int(metrics.unitsPerEm),
          );

        /* Set editor text based on measurements */
        App.dispatch(
          app,
          SetEditorFont(
            EditorFont.create(
              ~fontFile=fontFamily,
              ~fontSize,
              ~measuredWidth=glyph.advance / 64,
              ~measuredHeight=actualHeight,
              (),
            ),
          ),
        );
      },
      _ => prerr_endline("setFont: Failed to load font " ++ fontFamily),
    );

  setFont("FiraCode-Regular.ttf", 14);

  let commands = Core.Keybindings.get();

  let inputHandler =
    Input.handle(~neovimHandler=neovimProtocol.input, ~commands);

  Reglfw.Glfw.glfwSetCharModsCallback(w.glfwWindow, (_w, codepoint, mods) =>
    switch (Input.charToCommand(codepoint, mods)) {
    | None => ()
    | Some(v) =>
      inputHandler(~state=App.getState(app), v)
      |> List.iter(App.dispatch(app))
    }
  );

  Reglfw.Glfw.glfwSetKeyCallback(
    w.glfwWindow, (_w, key, _scancode, buttonState, mods) =>
    switch (Input.keyPressToCommand(key, buttonState, mods)) {
    | None => ()
    | Some(v) =>
      inputHandler(~state=App.getState(app), v)
      |> List.iter(App.dispatch(app))
    }
  );

  let _ =
    Tick.interval(
      _ => {
        nvimApi.pump();
        Oni_Core.TextmateClient.pump(tmClient);
      },
      Seconds(0.),
    );

  /* let _ = */
  /*   Event.subscribe(nvimApi.onNotification, n => */
  /*     prerr_endline( */
  /*       "Raw Notification: " */
  /*       ++ n.notificationType */
  /*       ++ " | " */
  /*       ++ Msgpck.show(n.payload), */
  /*     ) */
  /*   ); */

  let _ =
    Event.subscribe(
      neovimProtocol.onNotification,
      n => {
        open Core.Actions;
        let msg =
          switch (n) {
          | OniCommand("oni.editorView.scrollToCursor") =>
            EditorScrollToCursorCentered
          | OniCommand("oni.editorView.scrollToCursorTop") =>
            EditorScrollToCursorTop
          | OniCommand("oni.editorView.scrollToCursorBottom") =>
            EditorScrollToCursorBottom
          | OniCommand("oni.editorView.moveCursorToTop") =>
            EditorMoveCursorToTop(neovimProtocol.moveCursor)
          | OniCommand("oni.editorView.moveCursorToMiddle") =>
            EditorMoveCursorToMiddle(neovimProtocol.moveCursor)
          | OniCommand("oni.editorView.moveCursorToBottom") =>
            EditorMoveCursorToBottom(neovimProtocol.moveCursor)
          | ModeChanged("normal") => ChangeMode(Normal)
          | ModeChanged("insert") => ChangeMode(Insert)
          | ModeChanged("replace") => ChangeMode(Replace)
          | ModeChanged("visual") => ChangeMode(Visual)
          | ModeChanged("operator") => ChangeMode(Operator)
          | ModeChanged("cmdline_normal") => ChangeMode(Commandline)
          | TablineUpdate(tabs) => TablineUpdate(tabs)
          | ModeChanged(_) => ChangeMode(Other)
          | CursorMoved(c) =>
            CursorMove(
              Core.Types.BufferPosition.create(c.cursorLine, c.cursorColumn),
            )
          | BufferWritePost({activeBufferId, _}) =>
            BufferWritePost({
              bufferId: activeBufferId,
              buffers: NeovimBuffer.getBufferList(nvimApi),
            })
          | TextChangedI({activeBufferId, modified, _}) =>
            TextChangedI({activeBufferId, modified})
          | TextChanged({activeBufferId, modified, _}) =>
            TextChanged({activeBufferId, modified})
          | BufferEnter({activeBufferId, _}) =>
            neovimProtocol.bufAttach(activeBufferId);
            BufferEnter({
              bufferId: activeBufferId,
              buffers: NeovimBuffer.getBufferList(nvimApi),
            });
          | BufferDelete(bd) =>
            BufferDelete({
              buffers: NeovimBuffer.getBufferList(nvimApi),
              bufferId: bd.activeBufferId,
            })
          | BufferLines(bc) =>
            BufferUpdate(
              Core.Types.BufferUpdate.create(
                ~id=bc.id,
                ~startLine=bc.firstLine,
                ~endLine=bc.lastLine,
                ~lines=bc.lines,
                ~version=bc.changedTick,
                (),
              ),
            )
          | WildmenuShow(w) => WildmenuShow(w)
          | WildmenuHide(w) => WildmenuHide(w)
          | WildmenuSelected(s) => WildmenuSelected(s)
          | CommandlineUpdate(u) => CommandlineUpdate(u)
          | CommandlineShow(c) => CommandlineShow(c)
          | CommandlineHide(c) => CommandlineHide(c)
          | _ => Noop
          };

        App.dispatch(app, msg);

        /* TODO:
         * Refactor this into a middleware concept, like Redux */
        switch (msg) {
        | SetEditorFont(_)
        | SetEditorSize(_)
        | Core.Actions.BufferUpdate(_)
        | Core.Actions.BufferEnter(_) =>
          App.dispatch(app, RecalculateEditorView)
        | _ => ()
        };
        /* prerr_endline("Protocol Notification: " ++ Notification.show(n)); */

        /* TODO:
         * Refactor this into _another_ middleware
         */
        switch (msg) {
        | BufferUpdate(bc) =>
          Core.TextmateClient.notifyBufferUpdate(tmClient, bc)
        | _ => ()
        };
      },
    );
  ();
};

/* Let's get this party started! */
App.startWithState(state, Core.Reducer.reduce, init);
