/*
 * IndentationStoreConnector.re
 *
 * This implements an updater (reducer + side effects) for managing indentation
 */

open Oni_Core;
open Oni_Model;

let start = () => {
  /*
   * This effect checks if auto-indent should be applied,
   * and if so, applies it.
   *
   * Otherwise, it sets indentation values for a buffer,
   * if it hasn't been done yet.
   */
  let checkIndentationEffect = (state, bufferId) =>
    Isolinear.Effect.createWithDispatch(
      ~name="indentation.detectAndSet", dispatch => {
      open State;
      let buffer = Buffers.getBuffer(bufferId, state.buffers);
      switch (buffer) {
      | None => ()
      | Some(b) =>
        if (!Buffer.isIndentationSet(b)) {
          let indentationSettings =
            if (Configuration.getValue(
                  c => c.editorDetectIndentation,
                  state.configuration,
                )) {
              let f = Buffer.getLine(b);
              let count = Buffer.getNumberOfLines(b);

              let defaultInsertSpaces =
                Configuration.getValue(
                  c => c.editorInsertSpaces,
                  state.configuration,
                );
              let defaultIndentSize =
                Configuration.getValue(
                  c => c.editorIndentSize,
                  state.configuration,
                );

              let guess =
                IndentationGuesser.guessIndentation(
                  ~f,
                  count,
                  defaultIndentSize,
                  defaultInsertSpaces,
                );

              let size = switch (guess.mode) {
              | Tabs => 
                  Configuration.getValue(
                    c => c.editorTabSize,
                    state.configuration,
                  )
              | Spaces => guess.size
              };

              IndentationSettings.create(
                ~mode=guess.mode,
                ~size=size,
                ~tabSize=size,
                (),
              );
            } else {
              IndentationSettings.ofConfiguration(state.configuration);
            };

          dispatch(
            Actions.BufferSetIndentation(bufferId, indentationSettings),
          );
        } else {
          ();
        }
      };
    });

  let updater = (state: State.t, action: Actions.t) => {
    switch (action) {
    | Actions.BufferEnter(bu) => (
        state,
        checkIndentationEffect(state, bu.id),
      )
    | _ => (state, Isolinear.Effect.none)
    };
  };

  updater;
};
