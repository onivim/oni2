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

/* The 'main' function for our app */
let init = app => {
  let w =
    App.createWindow(
      ~createOptions={
        ...Window.defaultCreateOptions,
        vsync: false,
        maximized: false,
        icon: Some("logo.png"),
      },
      app,
      "Oni2",
    );

  let initVimPath = Revery.Environment.getExecutingDirectory() ++ "init.vim";
  Core.Log.debug("initVimPath: " ++ initVimPath);

  let setup: Oni_Core.Setup.t = Oni_Core.Setup.init();

  let nvim =
    NeovimProcess.start(
      ~neovimPath=setup.neovimPath,
      ~args=[|"-u", initVimPath, "--embed"|],
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

  let onScopeLoaded = s => prerr_endline("SCOPE LOADED: " ++ s);
  let onColorMap = cm =>
    App.dispatch(app, Core.Actions.SyntaxHighlightColorMap(cm));

  let tmClient =
    Oni_Core.TextmateClient.start(
      ~onScopeLoaded,
      ~onColorMap,
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

        let derp = float_of_int(glyph.advance) /. 64.;
        print_endline ("DERP: " ++ string_of_float(derp));

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

  setFont("FiraCode-Regular.ttf", 12);

  let _ =
    Event.subscribe(
      w.onKeyPress,
      event => {
        let c = event.character;
        neovimProtocol.input(c);
      },
    );

  let _ =
    Event.subscribe(
      w.onKeyDown,
      event => {
        let _ =
          switch (event.key, event.shiftKey, event.ctrlKey) {
          | (Key.KEY_TAB, true, _) =>
            ignore(neovimProtocol.input("<S-TAB>"))
          | (Key.KEY_BACKSPACE, _, _) =>
            ignore(neovimProtocol.input("<BS>"))
          | (Key.KEY_ENTER, _, _) => ignore(neovimProtocol.input("<CR>"))
          | (Key.KEY_ESCAPE, _, _) => ignore(neovimProtocol.input("<ESC>"))
          | (Key.KEY_TAB, _, _) => ignore(neovimProtocol.input("<TAB>"))
          | (Key.KEY_RIGHT_SHIFT, _, _)
          | (Key.KEY_LEFT_SHIFT, _, _) =>
            ignore(neovimProtocol.input("<SHIFT>"))
          | (Key.KEY_UP, _, _) => ignore(neovimProtocol.input("<UP>"))
          | (Key.KEY_LEFT, _, _) => ignore(neovimProtocol.input("<LEFT>"))
          | (Key.KEY_RIGHT, _, _) => ignore(neovimProtocol.input("<RIGHT>"))
          | (Key.KEY_DOWN, _, _) => ignore(neovimProtocol.input("<DOWN>"))
          | _ => ()
          };
        ();
      },
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
        | Core.Actions.BufferUpdate(_)
        | Core.Actions.BufferEnter(_) =>
          App.dispatch(app, RecalculateEditorView)
        | _ => ()
        };
        /* prerr_endline("Protocol Notification: " ++ Notification.show(n)); */
      },
    );
  ();
};

/* Let's get this party started! */
App.startWithState(Core.State.create(), Core.Reducer.reduce, init);
