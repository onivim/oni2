/*
 * LifecycleStoreConnector.re
 *
 * This manages side-effects related to the lifecycle.
 * - Handling quit cleanup
 */

open Oni_Model;

let start = quit => {
  let quitAllEffect = (state: State.t, force) => {
    let handlers = state.lifecycle.onQuitFunctions;

    let anyModified = Buffers.anyModified(state.buffers);
    let canClose = force || !anyModified;

    Isolinear.Effect.createWithDispatch(~name="lifecycle.quitAll", dispatch =>
      if (canClose) {
        dispatch(Actions.ReallyQuitting);
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
        if (Feature_Editor.Editor.getBufferId(editor) == bufferMeta.id) {
          if (force || !bufferMeta.modified) {
            dispatch(
              Actions.ViewCloseEditor(Feature_Editor.Editor.getId(editor)),
            );
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
        {...state, modal: Some(Feature_Modals.unsavedBuffersWarning)},
        Isolinear.Effect.none,
      )

    | _ => (state, Isolinear.Effect.none)
    };
  };

  updater;
};
