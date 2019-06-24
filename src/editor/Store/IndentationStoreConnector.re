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
            if (state.configuration.editorDetectIndentation) {
              let f = Buffer.getLine(b);
              let count = Buffer.getNumberOfLines(b);

              print_endline(
                "Number of lines checking: " ++ string_of_int(count),
              );

              let defaultInsertSpaces = state.configuration.editorInsertSpaces;
              let defaultIndentSize = state.configuration.editorIndentSize;

              let guess =
                IndentationGuesser.guessIndentation(
                  ~f,
                  count,
                  defaultIndentSize,
                  defaultInsertSpaces,
                );

              IndentationSettings.create(
                ~mode=guess.mode,
                ~size=guess.size,
                ~tabSize=state.configuration.editorTabSize,
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
      print_endline("Dispatching detect and set!");
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
