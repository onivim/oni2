
/*
 * StoreThread.re
 *
 * This is the 'state management' piece of Oni2. 
 * 
 * The state updates are run in a parallel thread to the rendering,
 * so that we can eek out as much perf as we can in this architecture.
 */

open Rench;
open Oni_Neovim;

module Core = Oni_Core;
module Extensions = Oni_Extensions;
module Model = Oni_model;

open Oni_Extensions;


let start = (
   ~setup: Core.Setup.t,
   ~executingDirectory,
   ~onStateChanged,
   (),
) => {

    let state = Model.State.create();

  let (dispatch) = Isolinear.Store.create(
      ~initialState=state,
      ~reducer=Model.Reducer.reduce,
  );
    
  let initVimPath =
    Path.join(executingDirectory, "init.vim");
  Core.Log.debug("initVimPath: " ++ initVimPath);

  let extensions = ExtensionScanner.scan(setup.bundledExtensionsPath);

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

  let extHostClient =
    Extensions.ExtensionHostClient.start(~onClosed=onExtHostClosed, setup);

  Extensions.TextmateClient.setTheme(tmClient, defaultThemePath);

  neovimProtocol.uiAttach();



  let _ =
    Tick.interval(
      _ => {
        nvimApi.pump();
        Extensions.TextmateClient.pump(tmClient);
        Extensions.ExtensionHostClient.pump(extHostClient);
      },
      Seconds(0.),
    );


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

        dispatch(msg);

        /* TODO:
         * Refactor this into a middleware concept, like Redux */
        switch (msg) {
        | SetEditorFont(_)
        | SetEditorSize(_)
        | Model.Actions.BufferUpdate(_)
        | Model.Actions.BufferEnter(_) =>
          dispatch(RecalculateEditorView)
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
}
