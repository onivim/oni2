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
module Extensions = Oni_Extensions;
module Model = Oni_Model;

open Oni_Extensions;

/**
   This allows a stack trace to be printed when exceptions occur
 */
switch (Sys.getenv_opt("ONI2_DEBUG")) {
| Some(_) => Printexc.record_backtrace(true) |> ignore
| None => ()
};

let state = Model.State.create();
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

  let setup = Core.Setup.init();
  let cliOptions = Cli.parse(setup);
  Sys.chdir(cliOptions.folder);

  let initVimPath =
    Path.join(Revery.Environment.getExecutingDirectory(), "init.vim");
  Core.Log.debug("initVimPath: " ++ initVimPath);

  let extensions = ExtensionScanner.scan(setup.bundledExtensionsPath);

  let developmentExtensions =
    switch (setup.developmentExtensionsPath) {
    | Some(p) =>
      let ret = ExtensionScanner.scan(p);
      ret;
    | None => []
    };

  let extensions = [extensions, developmentExtensions] |> List.flatten;

  let languageInfo = Model.LanguageInfo.ofExtensions(extensions);

  Core.Log.debug(
    "-- Discovered: "
    ++ string_of_int(List.length(extensions))
    ++ " extensions",
  );

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

  let _ =
    Event.subscribe(nvim.onClose, code =>
      if (code === 0) {
        App.quit(0);
      } else {
        ();
          /* TODO: What to do in case Neovim crashes? */
      }
    );

  let nvimApi = NeovimApi.make(msgpackTransport);
  let neovimProtocol = NeovimProtocol.make(nvimApi);

  let defaultThemePath =
    setup.bundledExtensionsPath ++ "/onedark-pro/themes/OneDark-Pro.json";

  let onScopeLoaded = s => prerr_endline("Scope loaded: " ++ s);
  let onColorMap = cm =>
    App.dispatch(app, Model.Actions.SyntaxHighlightColorMap(cm));

  let onTokens = tr =>
    App.dispatch(app, Model.Actions.SyntaxHighlightTokens(tr));

  let grammars = Model.LanguageInfo.getGrammars(languageInfo);

  let tmClient =
    Extensions.TextmateClient.start(
      ~onScopeLoaded,
      ~onColorMap,
      ~onTokens,
      setup,
      grammars,
    );

  let onExtHostClosed = () => print_endline("ext host closed");

  let extensionInfo =
    extensions
    |> List.map(ext =>
         Extensions.ExtensionHostInitData.ExtensionInfo.ofScannedExtension(
           ext,
         )
       );

  let onMessage = (scope, method, args) => {
    switch (scope, method, args) {
    | (
        "MainThreadStatusBar",
        "$setEntry",
        [
          `Int(id),
          _,
          `String(text),
          _,
          _,
          _,
          `Int(alignment),
          `Int(priority),
        ],
      ) =>
      App.dispatch(
        app,
        Model.Actions.StatusBarAddItem(
          Model.StatusBarModel.Item.create(
            ~id,
            ~text,
            ~alignment=Model.StatusBarModel.Alignment.ofInt(alignment),
            ~priority,
            (),
          ),
        ),
      );
      Ok(None);
    | _ => Ok(None)
    };
  };

  let initData = ExtensionHostInitData.create(~extensions=extensionInfo, ());
  let extHostClient =
    Extensions.ExtensionHostClient.start(
      ~initData,
      ~onClosed=onExtHostClosed,
      ~onMessage,
      setup,
    );

  Extensions.TextmateClient.setTheme(tmClient, defaultThemePath);

  let render = () => {
    let state: Model.State.t = App.getState(app);
    GlobalContext.set({
      state,
      notifySizeChanged: (~width, ~height, ()) =>
        App.dispatch(
          app,
          Model.Actions.SetEditorSize(
            Core.Types.EditorSize.create(
              ~pixelWidth=width,
              ~pixelHeight=height,
              (),
            ),
          ),
        ),
      editorScroll: (~deltaY, ()) =>
        App.dispatch(app, Model.Actions.EditorScroll(deltaY)),
      openFile: neovimProtocol.openFile,
      closeFile: neovimProtocol.closeFile,
      dispatch: App.dispatch(app),
    });

    <Root state />;
  };

  UI.start(w, render);

  neovimProtocol.uiAttach();

  let setFont = (fontFamily, fontSize) => {
    let scaleFactor =
      Window.getDevicePixelRatio(w)
      *. float_of_int(Window.getScaleFactor(w));

    let adjSize = int_of_float(float_of_int(fontSize) *. scaleFactor +. 0.5);

    Fontkit.fk_new_face(
      Revery.Environment.getExecutingDirectory() ++ fontFamily,
      adjSize,
      font => {
        open Oni_Model.Actions;
        open Oni_Core.Types;

        /* Measure text */
        let shapedText = Fontkit.fk_shape(font, "H");
        let firstShape = shapedText[0];
        let glyph = Fontkit.renderGlyph(font, firstShape.glyphId);

        let metrics = Fontkit.fk_get_metrics(font);
        let actualHeight =
          float_of_int(fontSize)
          *. float_of_int(metrics.height)
          /. float_of_int(metrics.unitsPerEm);

        /* Set editor text based on measurements */
        App.dispatch(
          app,
          SetEditorFont(
            EditorFont.create(
              ~fontFile=fontFamily,
              ~fontSize,
              ~measuredWidth=
                float_of_int(glyph.advance) /. (64. *. scaleFactor),
              ~measuredHeight=actualHeight,
              (),
            ),
          ),
        );
      },
      _ => prerr_endline("setFont: Failed to load font " ++ fontFamily),
    );
  };

  setFont("FiraCode-Regular.ttf", 14);

  let commands = Core.Keybindings.get();

  Model.Menu.addEffects({
    openFile: neovimProtocol.openFile,
    getCurrentDir: neovimProtocol.getCurrentDir,
  })
  |> App.dispatch(app)
  |> ignore;

  let inputHandler = Input.handle(~api=neovimProtocol, ~commands);

  /**
     The key handlers return (keyPressedString, shouldOniListen)
     i.e. if ctrl or alt or cmd were pressed then Oni2 should listen
     /respond to commands otherwise if input is alphabetical AND
     a revery element is focused oni2 should defer to revery
   */
  let keyEventListener = key =>
    switch (key, Focus.focused) {
    | (None, _) => ()
    | (Some((k, true)), {contents: Some(_)})
    | (Some((k, _)), {contents: None}) =>
      inputHandler(~state=App.getState(app), k)
      |> List.iter(App.dispatch(app))
    | (Some((_, false)), {contents: Some(_)}) => ()
    };

  Event.subscribe(w.onKeyDown, keyEvent =>
    Input.keyPressToCommand(keyEvent) |> keyEventListener
  )
  |> ignore;

  Reglfw.Glfw.glfwSetCharModsCallback(w.glfwWindow, (_w, codepoint, mods) =>
    Input.charToCommand(codepoint, mods) |> keyEventListener
  );

  let _ =
    Tick.interval(
      _ => {
        nvimApi.pump();
        Extensions.TextmateClient.pump(tmClient);
        Extensions.ExtensionHostClient.pump(extHostClient);
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
        open Model.Actions;
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
        | Model.Actions.BufferUpdate(_)
        | Model.Actions.BufferEnter(_) =>
          App.dispatch(app, RecalculateEditorView)
        | _ => ()
        };
        /* prerr_endline("Protocol Notification: " ++ Notification.show(n)); */

        /* TODO:
         * Refactor this into _another_ middleware
         */
        switch (msg) {
        | BufferUpdate(bc) =>
          let bufferId = bc.id;
          let state = App.getState(app);
          let buffer = Model.BufferMap.getBuffer(bufferId, state.buffers);

          switch (buffer) {
          | None => ()
          | Some(buffer) =>
            switch (Model.Buffer.getMetadata(buffer).filePath) {
            | None => ()
            | Some(v) =>
              let extension = Path.extname(v);
              switch (
                Model.LanguageInfo.getScopeFromExtension(
                  languageInfo,
                  extension,
                )
              ) {
              | None => ()
              | Some(scope) =>
                Extensions.TextmateClient.notifyBufferUpdate(
                  tmClient,
                  scope,
                  bc,
                )
              };
            }
          };

        | _ => ()
        };
      },
    );

  List.iter(
    p => neovimProtocol.openFile(~path=p, ()),
    cliOptions.filesToOpen,
  );

  ();
};

/* Let's get this party started! */
App.startWithState(state, Model.Reducer.reduce, init);
