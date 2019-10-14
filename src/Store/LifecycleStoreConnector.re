/*
 * LifecycleStoreConnector.re
 *
 * This manages side-effects related to the lifecycle.
 * - Handling quit cleanup
 */

module Core = Oni_Core;
module Model = Oni_Model;

let start = quit => {
  let (stream, dispatch) = Isolinear.Stream.create();

  let quitAllEffect = (state: Model.State.t, force) => {
    let handlers = state.lifecycle.onQuitFunctions;

    let anyModified = Model.Buffers.anyModified(state.buffers);
    let canClose = force || !anyModified;

    Isolinear.Effect.create(~name="lifecycle.quit", () =>
      if (canClose) {
        List.iter(h => h(), handlers);
        quit(0);
      }
    );
  };

  let quitBufferEffect = (state: Model.State.t, buffer: Vim.Buffer.t, force) => {
    Isolinear.Effect.create(~name="lifecycle.quitBuffer", () => {
      let editorGroup = Model.Selectors.getActiveEditorGroup(state);
      switch (Model.Selectors.getActiveEditor(editorGroup)) {
      | None => ()
      | Some(editor) =>
        let bufferMeta = Vim.BufferMetadata.ofBuffer(buffer);
        if (editor.bufferId == bufferMeta.id) {
          if (force || !bufferMeta.modified) {
            dispatch(Model.Actions.ViewCloseEditor(editor.editorId));
          };
        };
      };
    });
  };

  let updater = (state: Model.State.t, action) => {
    switch (action) {
    | Model.Actions.QuitBuffer(buffer, force) => (
        state,
        quitBufferEffect(state, buffer, force),
      )
    | Model.Actions.Quit(force) => (state, quitAllEffect(state, force))
    | _ => (state, Isolinear.Effect.none)
    };
  };

  (updater, stream);
};
