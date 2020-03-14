/*
 * IndentationStoreConnector.re
 *
 * This implements an updater (reducer + side effects) for managing indentation
 */

open Oni_Core;
open Oni_Model;

let start = () => {
  let detectIndentationEffect = (state: State.t, buffer) =>
    Isolinear.Effect.createWithDispatch(
      ~name="indentation.detectAndSet", dispatch => {
      let count = Buffer.getNumberOfLines(buffer);

      let defaultInsertSpaces =
        Configuration.getValue(
          c => c.editorInsertSpaces,
          state.configuration,
        );
      let defaultIndentSize =
        Configuration.getValue(c => c.editorIndentSize, state.configuration);

      let guess =
        IndentationGuesser.guessIndentation(
          ~f=line => Buffer.getLine(line, buffer) |> BufferLine.raw,
          count,
          defaultIndentSize,
          defaultInsertSpaces,
        );

      let size =
        switch (guess.mode) {
        | Tabs =>
          Configuration.getValue(c => c.editorTabSize, state.configuration)
        | Spaces => guess.size
        };

      dispatch(
        Actions.BufferSetIndentation(
          Buffer.getId(buffer),
          IndentationSettings.create(
            ~mode=guess.mode,
            ~size,
            ~tabSize=size,
            (),
          ),
        ),
      );
    });

  let setIndentationEffect = (state: State.t, buffer) =>
    Isolinear.Effect.createWithDispatch(
      ~name="indentation.setConfigured", dispatch =>
      dispatch(
        Actions.BufferSetIndentation(
          Buffer.getId(buffer),
          IndentationSettings.ofConfiguration(state.configuration),
        ),
      )
    );

  /*
   * This effect checks if auto-indent should be applied,
   * and if so, applies it.
   *
   * Otherwise, it sets indentation values for a buffer,
   * if it hasn't been done yet.
   */
  let checkIndentationEffect = (state: State.t, bufferId) => {
    let maybeBuffer = Buffers.getBuffer(bufferId, state.buffers);
    switch (maybeBuffer) {
    | Some(buffer) =>
      if (!Buffer.isIndentationSet(buffer)) {
        if (Configuration.getValue(
              c => c.editorDetectIndentation,
              state.configuration,
            )) {
          detectIndentationEffect(state, buffer);
        } else {
          setIndentationEffect(state, buffer);
        };
      } else {
        Isolinear.Effect.none;
      }
    | None => Isolinear.Effect.none
    };
  };

  let updater = (state: State.t, action: Actions.t) => {
    switch (action) {
    | Actions.BufferEnter(metadata, _) => (
        state,
        checkIndentationEffect(state, metadata.id),
      )

    | Actions.Command("editor.action.detectIndentation") =>
      switch (Selectors.getActiveBuffer(state)) {
      | Some(buffer) => (state, detectIndentationEffect(state, buffer))
      | None => (state, Isolinear.Effect.none)
      }

    | _ => (state, Isolinear.Effect.none)
    };
  };

  updater;
};
