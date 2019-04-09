/*
 * NeovimStoreConnector.re
 *
 * This module connects Neovim to the Store:
 * - Translates incoming neovim notifications into Actions
 * - Translates Actions into Effects that should run against Neovim
 */

open Rench;
open Revery;

open Oni_Neovim;

module Core = Oni_Core;
module Extensions = Oni_Extensions;
module Model = Oni_Model;

let start = (executingDirectory, setup: Core.Setup.t, cli: Core.Cli.t) => {
  let initVimPath = Path.join(executingDirectory, "init.vim");
  Core.Log.debug("initVimPath: " ++ initVimPath);

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

  /* let _ = */
  /*   Event.subscribe(nvimApi.onNotification, n => */
  /*     prerr_endline( */
  /*       "Raw Notification: " */
  /*       ++ n.notificationType */
  /*       ++ " | " */
  /*       ++ Msgpck.show(n.payload), */
  /*     ) */
  /*   ); */
  /* }, */
  /* ); */

  let neovimProtocol = NeovimProtocol.make(nvimApi);
  neovimProtocol.uiAttach();

  let pumpEffect =
    Isolinear.Effect.create(~name="neovim.pump", () => nvimApi.pump());
  let inputEffect = key =>
    Isolinear.Effect.create(~name="neovim.input", () =>
      neovimProtocol.input(key)
    );

  let openFileByPathEffect = filePath =>
    Isolinear.Effect.create(~name="neovim.openFileByPath", () =>
      neovimProtocol.openFile(~path=filePath, ())
    );

  let openFileByIdEffect = id =>
    Isolinear.Effect.create(~name="neovim.openFileById", () =>
      neovimProtocol.openFile(~id, ())
    );

  let closeFileByIdEffect = id =>
    Isolinear.Effect.create(~name="neovim.closeFileByIdEffect", () =>
      neovimProtocol.closeFile(~id, ())
    );

  let openConfigFileEffect = filePath =>
    Isolinear.Effect.create(~name="neovim.openConfigFile", () =>
      switch (Core.Filesystem.createOniConfigFile(filePath)) {
      | Ok(path) => neovimProtocol.openFile(~path, ())
      | Error(e) => print_endline(e)
      }
    );

  let requestVisualRangeUpdateEffect =
    Isolinear.Effect.create(~name="neovim.refreshVisualRange", () =>
      neovimProtocol.requestVisualRangeUpdate()
    );

  let updater = (state: Model.State.t, action) => {
    switch (action) {
    | Model.Actions.Init =>
      let filesToOpen = cli.filesToOpen;
      let openFileEffects =
        filesToOpen
        |> List.map(openFileByPathEffect)
        |> Isolinear.Effect.batch;
      (state, openFileEffects);
    | Model.Actions.OpenFileByPath(path) => (
        state,
        openFileByPathEffect(path),
      )
    | Model.Actions.OpenFileById(id) => (state, openFileByIdEffect(id))
    | Model.Actions.CloseFileById(id) => (state, closeFileByIdEffect(id))
    | Model.Actions.OpenConfigFile(path) => (
        state,
        openConfigFileEffect(path),
      )
    | Model.Actions.CursorMove(_) => (
        state,
        state.mode === Core.Types.Mode.Visual
          ? requestVisualRangeUpdateEffect : Isolinear.Effect.none,
      )
    | Model.Actions.ChangeMode(_) => (state, requestVisualRangeUpdateEffect)
    | Model.Actions.Tick => (state, pumpEffect)
    | Model.Actions.KeyboardInput(s) => (state, inputEffect(s))
    | _ => (state, Isolinear.Effect.none)
    };
  };

  let stream =
    Isolinear.Stream.ofDispatch(send => {
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
              | VisualRangeUpdate(vr) => SelectionChanged(vr)
              | CursorMoved(c) =>
                CursorMove(
                  Core.Types.Position.create(c.cursorLine, c.cursorColumn),
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
                  Core.Types.BufferUpdate.createFromZeroBasedIndices(
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

            send(msg);
          },
        );
      ();
    });

  (updater, stream);
};
