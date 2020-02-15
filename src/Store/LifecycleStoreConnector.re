/*
 * LifecycleStoreConnector.re
 *
 * This manages side-effects related to the lifecycle.
 * - Handling quit cleanup
 */

open Oni_Model;

let start = quit => {
  let saveAllAndQuitEffect =
    Isolinear.Effect.create(~name="lifecycle.saveAllAndQuit", () => {
      Vim.input("<ESC>") |> (ignore: list(Vim.Cursor.t) => unit);
      Vim.input("<ESC>") |> (ignore: list(Vim.Cursor.t) => unit);
      Vim.input(":") |> (ignore: list(Vim.Cursor.t) => unit);
      Vim.input("x") |> (ignore: list(Vim.Cursor.t) => unit);
      Vim.input("a") |> (ignore: list(Vim.Cursor.t) => unit);
      Vim.input("<CR>") |> (ignore: list(Vim.Cursor.t) => unit);
    });

  let quitAllEffect = (state: State.t, force) => {
    let handlers = state.lifecycle.onQuitFunctions;

    let anyModified = Buffers.anyModified(state.buffers);
    let canClose = force || !anyModified;

    Isolinear.Effect.create(~name="lifecycle.quitAll", () =>
      if (canClose) {
        List.iter(h => h(), handlers);
        quit(0);
      }
    );
  };

  let quitBufferEffect = (state: State.t, buffer: Vim.Buffer.t, force) => {
    Isolinear.Effect.createWithDispatch(~name="lifecycle.quitBuffer", dispatch => {
      let editorGroup = Selectors.getActiveEditorGroup(state);
      switch (Selectors.getActiveEditor(editorGroup)) {
      | None => ()
      | Some(editor) =>
        let bufferMeta = Vim.BufferMetadata.ofBuffer(buffer);
        if (editor.bufferId == bufferMeta.id) {
          if (force || !bufferMeta.modified) {
            dispatch(Actions.ViewCloseEditor(editor.editorId));
          };
        };
      };
    });
  };

  let updater = (state: State.t, action) => {
    switch (action) {
    | Actions.QuitBuffer(buffer, force) => (
        state,
        quitBufferEffect(state, buffer, force),
      )

    | Actions.Quit(force) => (state, quitAllEffect(state, force))

    | WindowCloseBlocked => (
        {...state, modal: Some(Oni_UI.Modals.unsavedBuffersWarning)}
        |> FocusManager.push(Focus.Modal),
        Isolinear.Effect.none,
      )

    | WindowCloseDiscardConfirmed => (
        {...state, modal: None} |> FocusManager.pop(Focus.Modal),
        quitAllEffect(state, true),
      )

    | WindowCloseSaveAllConfirmed => (
        {...state, modal: None} |> FocusManager.pop(Focus.Modal),
        saveAllAndQuitEffect,
      )

    | WindowCloseCanceled => (
        {...state, modal: None} |> FocusManager.pop(Focus.Modal),
        Isolinear.Effect.none,
      )

    | _ => (state, Isolinear.Effect.none)
    };
  };

  updater;
};
