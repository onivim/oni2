/*
 * vimStoreConnector.re
 *
 * This module connects vim to the Store:
 * - Translates incoming vim notifications into Actions
 * - Translates Actions into Effects that should run against vim
 */

open Rench;
open Revery;

module Core = Oni_Core;
module Extensions = Oni_Extensions;
module Model = Oni_Model;

let start = (executingDirectory, setup: Core.Setup.t, cli: Core.Cli.t) => {
  let initVimPath = Path.join(executingDirectory, "init.vim");
  Core.Log.debug("initVimPath: " ++ initVimPath);

  let (stream, dispatch) = Isolinear.Stream.create();

  let _ =
    Vim.Mode.onChanged(newMode => {
      print_endline("Mode changed!");
      dispatch(Model.Actions.ChangeMode(newMode));
    });

  let _ =
    Vim.Cursor.onMoved(newPosition => {
      let cursorPos =
        Core.Types.Position.createFromOneBasedIndices(
          newPosition.line,
          newPosition.column + 1,
        );
      dispatch(Model.Actions.CursorMove(cursorPos));
      /* Printf.printf("Cursor position - line: %d column: %d\n", newPosition.line, newPosition.column); */
    });

  /* let _ = */
  /*   Vim.Buffer.onEnter((buf) => { */
  /*       let meta = Vim.BufferMetadata.ofBuffer(buf); */
  /*       dispatch(Model.Actions.BufferEnter(meta)); */
  /*   }); */

  let _ =
    Vim.AutoCommands.onDispatch((cmd, buf) =>
      switch (cmd) {
      | BufEnter =>
        print_endline("Dispatching buffer enter!");
        let meta = Vim.BufferMetadata.ofBuffer(buf);
        dispatch(Model.Actions.BufferEnter(meta));
      | _ => ()
      }
    );

  let _ =
    Vim.Buffer.onUpdate(update => {
        open Vim.BufferUpdate;
        open Core.Types;
        let bu = Core.Types.BufferUpdate.create(
            ~id=update.id,
            ~startLine=Index.OneBasedIndex(update.startLine),
            ~endLine=Index.OneBasedIndex(update.endLine),
            ~lines=update.lines,
            ~version=update.version,
            ()
        );

        dispatch(Model.Actions.BufferUpdate(bu));
    });

  let _ =
      Vim.CommandLine.onEnter((c) => {
        dispatch(Model.Actions.CommandlineShow(c.cmdType));
      });

  let _ =
      Vim.CommandLine.onUpdate((c) => {
        dispatch(Model.Actions.CommandlineUpdate(c));
      });

  let _ =
      Vim.CommandLine.onLeave(() => {
        dispatch(Model.Actions.CommandlineHide);
      });




  let initEffect = Isolinear.Effect.create(~name="vim.init", () => {
      Vim.init();
    });

  /* TODO: Move to init */
  /* let metadata = Vim.Buffer.getCurrent() */
  /* |> Vim.BufferMetadata.ofBuffer; */
  /* dispatch(Model.Actions.BufferEnter(metadata)); */

  let currentBufferId: ref(option(int)) = ref(None);
  let currentEditorId: ref(option(int)) = ref(None);

  let inputEffect = key =>
    Isolinear.Effect.create(~name="vim.input", () =>
      if (!String.equal(key, "<S-SHIFT>") && !String.equal(key, "<C->")) {
        print_endline("Sending key: " ++ key);
        Vim.input(key);
      }
    );

  let openFileByPathEffect = filePath =>
    Isolinear.Effect.create(~name="vim.openFileByPath", () =>
      Vim.Buffer.openFile(filePath) |> ignore
    );

  /* let registerQuitHandlerEffect = */
  /*   Isolinear.Effect.createWithDispatch( */
  /*     ~name="vim.registerQuitHandler", dispatch => */
  /*     dispatch(Model.Actions.RegisterQuitCleanup(quitCleanup)) */
  /*   ); */

  /**
   synchronizeEditorEffect checks the current state of the app:
   - open buffer
   - open editor

   If it is changed from the last time we 'synchronized', we
   push those changes to vim and record the latest state.

   This allows us to keep the buffer management in Onivim 2,
   and treat vim as an entity for manipulating a singular buffer.
   */
  let synchronizeEditorEffect = state =>
    Isolinear.Effect.create(~name="vim.synchronizeEditor", () => {
      let editor =
        Model.Selectors.getActiveEditorGroup(state)
        |> Model.Selectors.getActiveEditor;

      let editorBuffer = Model.Selectors.getActiveBuffer(state);
      switch (editorBuffer, currentBufferId^) {
      | (Some(editorBuffer), Some(v)) =>
        let id = Model.Buffer.getId(editorBuffer);
        if (id != v) {
          let buf = Vim.Buffer.getById(id);
          switch (buf) {
          | None => ()
          | Some(v) => Vim.Buffer.setCurrent(v)
          };
        };
      | (Some(editorBuffer), _) =>
        let id = Model.Buffer.getId(editorBuffer);
        let buf = Vim.Buffer.getById(id);
        switch (buf) {
        | None => ()
        | Some(v) => Vim.Buffer.setCurrent(v)
        };
      | _ => ()
      };

      let synchronizeCursorPosition = (editor: Model.Editor.t) => {
        Core.Types.
          /* TODO */
          /* vimProtocol.moveCursor( */
          /*   ~column=Index.toOneBasedInt(editor.cursorPosition.character), */
          /*   ~line=Index.toOneBasedInt(editor.cursorPosition.line), */
          /* ); */
          (currentEditorId := Some(editor.id));
      };

      switch (editor, currentEditorId^) {
      | (Some(e), Some(v)) when e.id != v => synchronizeCursorPosition(e)
      | (Some(e), _) => synchronizeCursorPosition(e)
      | _ => ()
      };
    });

  let updater = (state: Model.State.t, action) => {
    switch (action) {
    | Model.Actions.Init => (state, initEffect);
    | Model.Actions.OpenFileByPath(path) => (
        state,
        openFileByPathEffect(path),
      )
    /* | Model.Actions.CursorMove(_) => ( */
    /*     state, */
    /*     state.mode === Core.Types.Mode.Visual */
    /*       ? requestVisualRangeUpdateEffect : Isolinear.Effect.none, */
    /*   ) */
    | Model.Actions.BufferEnter(_) => (state, synchronizeEditorEffect(state))
    | Model.Actions.ViewSetActiveEditor(_) => (
        state,
        synchronizeEditorEffect(state),
      )
    | Model.Actions.ViewCloseEditor(_) => (
        state,
        synchronizeEditorEffect(state),
      )
    /* | Model.Actions.ChangeMode(_) => (state, requestVisualRangeUpdateEffect) */
    | Model.Actions.KeyboardInput(s) => (state, inputEffect(s))
    | _ => (state, Isolinear.Effect.none)
    };
  };

  /* let _ = */
  /*   Event.subscribe( */
  /*     vimProtocol.onNotification, */
  /*     n => { */
  /*       open Model.Actions; */
  /*       let msg = */
  /*         switch (n) { */
  /*         | OniCommand("oni.editorView.scrollToCursor") => */
  /*           EditorScrollToCursorCentered */
  /*         | OniCommand("oni.editorView.scrollToCursorTop") => */
  /*           EditorScrollToCursorTop */
  /*         | OniCommand("oni.editorView.scrollToCursorBottom") => */
  /*           EditorScrollToCursorBottom */
  /*         | OniCommand("oni.editorView.moveCursorToTop") => */
  /*           EditorMoveCursorToTop(vimProtocol.moveCursor) */
  /*         | OniCommand("oni.editorView.moveCursorToMiddle") => */
  /*           EditorMoveCursorToMiddle(vimProtocol.moveCursor) */
  /*         | OniCommand("oni.editorView.moveCursorToBottom") => */
  /*           EditorMoveCursreason-orToBottom(vimProtocol.moveCursor) */
  /*         | ModeChanged("normal") => ChangeMode(Normal) */
  /*         | ModeChanged("insert") => ChangeMode(Insert) */
  /*         | ModeChanged("replace") => ChangeMode(Replace) */
  /*         | ModeChanged("visual") => ChangeMode(Visual) */
  /*         | ModeChanged("operator") => ChangeMode(Operator) */
  /*         | ModeChanged("cmdline_normal") => ChangeMode(Commandline) */
  /*         | ModeChanged(_) => ChangeMode(Other) */
  /*         | VisualRangeUpdate(vr) => SelectionChanged(vr) */
  /*         | CursorMoved(c) => */
  /*           CursorMove( */
  /*             Core.Types.Position.create(c.cursorLine, c.cursorColumn), */
  /*           ) */
  /*         /1* | BufferWritePost({activeBufferId, _}) => *1/ */
  /*         /1*   let context = vimBuffer.getContext(nvimApi, activeBufferId); *1/ */
  /*         /1*   BufferSaved(context); *1/ */
  /*         | TextChanged({activeBufferId, _}) */
  /*         | TextChangedI({activeBufferId, _}) => */
  /*           BufferMarkDirty(activeBufferId) */
  /*         | BufferEnter({activeBufferId, _}) => */
  /*           /1* vimProtocol.bufAttach(activeBufferId); *1/ */

  /*           let context = vimBuffer.getContext(nvimApi, activeBufferId); */
  /*           currentBufferId := Some(activeBufferId); */
  /*           BufferEnter(context); */

  /*         | BufferDelete(_) => Noop */
  /*         | BufferLines(bc) => */
  /*           BufferUpdate( */
  /*             Core.Types.BufferUpdate.createFromZeroBasedIndices( */
  /*               ~id=bc.id, */
  /*               ~startLine=bc.firstLine, */
  /*               ~endLine=bc.lastLine, */
  /*               ~lines=bc.lines, */
  /*               ~version=bc.changedTick, */
  /*               (), */
  /*             ), */
  /*           ) */
  /*         | WildmenuShow(w) => WildmenuShow(w) */
  /*         | WildmenuHide(w) => WildmenuHide(w) */
  /*         | WildmenuSelected(s) => WildmenuSelected(s) */
  /*         | CommandlineUpdate(u) => CommandlineUpdate(u) */
  /*         | CommandlineShow(c) => CommandlineShow(c) */
  /*         | CommandlineHide(c) => CommandlineHide(c) */
  /*         | _ => Noop */
  /*         }; */

  /*       dispatch(msg); */
  /*     }, */
  /*   ); */

  (updater, stream);
};
