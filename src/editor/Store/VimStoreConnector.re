/*
 * vimStoreConnector.re
 *
 * This module connects vim to the Store:
 * - Translates incoming vim notifications into Actions
 * - Translates Actions into Effects that should run against vim
 */

module Core = Oni_Core;
module Extensions = Oni_Extensions;
module Model = Oni_Model;

let start = () => {
  let (stream, dispatch) = Isolinear.Stream.create();

  let _ =
    Vim.Mode.onChanged(newMode =>
      dispatch(Model.Actions.ChangeMode(newMode))
    );

  let _ =
    Vim.Cursor.onMoved(newPosition => {
      let cursorPos =
        Core.Types.Position.createFromOneBasedIndices(
          newPosition.line,
          newPosition.column + 1,
        );
      dispatch(Model.Actions.CursorMove(cursorPos));
    });

  let _ =
    Vim.Visual.onRangeChanged(vr => {
      open Vim.Range;
      open Vim.VisualRange;

      let {visualType, range} = vr;
      let {startPos, endPos} = range;
      let startColumn = startPos.column + 1;
      let endColumn = endPos.column + 1;
      let vr =
        Core.VisualRange.create(
          ~startLine=startPos.line,
          ~startColumn,
          ~endLine=endPos.line,
          ~endColumn,
          ~mode=visualType,
          (),
        );
      dispatch(SelectionChanged(vr));
    });

  let _ =
    Vim.Buffer.onEnter(buf => {
      let meta = {
        ...Vim.BufferMetadata.ofBuffer(buf),
        /*
             Set version to 0 so that a buffer update is processed.
             If not - we'd ignore the first buffer update that came through!
         */
        version: 0,
      };
      dispatch(Model.Actions.BufferEnter(meta));
    });

  let _ =
    Vim.Buffer.onUpdate(update => {
      open Vim.BufferUpdate;
      open Core.Types;
      let bu =
        Core.Types.BufferUpdate.create(
          ~id=update.id,
          ~startLine=Index.OneBasedIndex(update.startLine),
          ~endLine=Index.OneBasedIndex(update.endLine),
          ~lines=update.lines,
          ~version=update.version,
          (),
        );

      dispatch(Model.Actions.BufferUpdate(bu));
    });

  let _ =
    Vim.CommandLine.onEnter(c =>
      dispatch(Model.Actions.CommandlineShow(c.cmdType))
    );

  let _ =
    Vim.CommandLine.onUpdate(c =>
      dispatch(Model.Actions.CommandlineUpdate(c))
    );

  let _ =
    Vim.CommandLine.onLeave(() => dispatch(Model.Actions.CommandlineHide));

  let initEffect = Isolinear.Effect.create(~name="vim.init", () => Vim.init());

  /* TODO: Move to init */
  /* let metadata = Vim.Buffer.getCurrent() */
  /* |> Vim.BufferMetadata.ofBuffer; */
  /* dispatch(Model.Actions.BufferEnter(metadata)); */

  let currentBufferId: ref(option(int)) = ref(None);
  let currentEditorId: ref(option(int)) = ref(None);

  let inputEffect = key =>
    Isolinear.Effect.create(~name="vim.input", ()
      /* TODO: Fix these keypaths in libvim to not be blocking */
      =>
        if (!String.equal(key, "<S-SHIFT>")
            && !String.equal(key, "<C->")
            && !String.equal(key, "<A-C->")) {
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
        /* TODO */
        /* vimProtocol.moveCursor( */
        /*   ~column=Index.toOneBasedInt(editor.cursorPosition.character), */
        /*   ~line=Index.toOneBasedInt(editor.cursorPosition.line), */
        /* ); */
        currentEditorId := Some(editor.id);
      };

      switch (editor, currentEditorId^) {
      | (Some(e), Some(v)) when e.id != v => synchronizeCursorPosition(e)
      | (Some(e), _) => synchronizeCursorPosition(e)
      | _ => ()
      };
    });

  let updater = (state: Model.State.t, action) => {
    switch (action) {
    | Model.Actions.Init => (state, initEffect)
    | Model.Actions.OpenFileByPath(path) => (
        state,
        openFileByPathEffect(path),
      )
    | Model.Actions.BufferEnter(_) => (state, synchronizeEditorEffect(state))
    | Model.Actions.ViewSetActiveEditor(_) => (
        state,
        synchronizeEditorEffect(state),
      )
    | Model.Actions.ViewCloseEditor(_) => (
        state,
        synchronizeEditorEffect(state),
      )
    | Model.Actions.KeyboardInput(s) => (state, inputEffect(s))
    | _ => (state, Isolinear.Effect.none)
    };
  };

  (updater, stream);
};
