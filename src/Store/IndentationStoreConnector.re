/*
 * IndentationStoreConnector.re
 *
 * This implements an updater (reducer + side effects) for managing indentation
 */

open Oni_Core;
open Oni_Model;

let start = () => {
  //  let detectIndentationEffect = (state: State.t, buffer) =>
  //    Isolinear.Effect.createWithDispatch(
  //      ~name="indentation.detectAndSet", dispatch => {
  //      let count = Buffer.getNumberOfLines(buffer);
  //
  //      let defaultInsertSpaces =
  //        Configuration.getValue(
  //          c => c.editorInsertSpaces,
  //          state.configuration,
  //        );
  //      let defaultIndentSize =
  //        Configuration.getValue(c => c.editorIndentSize, state.configuration);
  //
  //      let guess =
  //        IndentationGuesser.guessIndentation(
  //          ~f=line => Buffer.getLine(line, buffer) |> BufferLine.raw,
  //          count,
  //          defaultIndentSize,
  //          defaultInsertSpaces,
  //        );
  //
  //      let size =
  //        switch (guess.mode) {
  //        | Tabs =>
  //          Configuration.getValue(c => c.editorTabSize, state.configuration)
  //        | Spaces => guess.size
  //        };
  //
  //      dispatch(
  //        Actions.Buffers(
  //          Feature_Buffers.Msg.indentationSet(
  //            ~bufferId=Buffer.getId(buffer),
  //            ~indentation=IndentationSettings.create(
  //              ~mode=guess.mode,
  //              ~size,
  //              ~tabSize=size,
  //              (),
  //            ),
  //          ),
  //        ),
  //      );
  //    });

  //  let setIndentationEffect = (state: State.t, buffer) =>
  //    Isolinear.Effect.createWithDispatch(
  //      ~name="indentation.setConfigured", dispatch =>
  //      dispatch(
  //        Actions.Buffers(
  //          Feature_Buffers.IndentationSet(
  //            Buffer.getId(buffer),
  //            IndentationSettings.ofConfiguration(state.configuration),
  //          ),
  //        ),
  //      )
  //    );

  /*
   * This effect checks if auto-indent should be applied,
   * and if so, applies it.
   *
   * Otherwise, it sets indentation values for a buffer,
   * if it hasn't been done yet.
   */
  //  let checkIndentationEffect = (state: State.t, buffer) =>
  //    if (Configuration.getValue(
  //          c => c.editorDetectIndentation,
  //          state.configuration,
  //        )) {
  //      detectIndentationEffect(state, buffer);
  //    } else {
  //      setIndentationEffect(state, buffer);
  //    };

  let updater = (state: State.t, action: Actions.t) => {
    switch (action) {
    // TODO: Bring back in another form
    //    | Actions.Buffers(Feature_Buffers.Entered({buffer, _})) =>
    //      let id = Oni_Core.Buffer.getId(buffer);
    //      switch (Feature_Buffers.get(id, state.buffers)) {
    //      | Some(buffer) when !Buffer.isIndentationSet(buffer) => (
    //          state,
    //          checkIndentationEffect(state, buffer),
    //        )
    //      | _ => (state, Isolinear.Effect.none)
    //      };
    //
    //    | Actions.Buffers(Feature_Buffers.Update({oldBuffer, newBuffer, _}))
    //        when
    //          Buffer.getNumberOfLines(oldBuffer) <= 1
    //          && Buffer.getNumberOfLines(newBuffer) > 2 => (
    //        state,
    //        checkIndentationEffect(state, newBuffer),
    //      )
    //
    //    | Actions.Command("editor.action.detectIndentation") =>
    //      switch (Selectors.getActiveBuffer(state)) {
    //      | Some(buffer) => (state, detectIndentationEffect(state, buffer))
    //      | None => (state, Isolinear.Effect.none)
    //      }

    | _ => (state, Isolinear.Effect.none)
    };
  };

  updater;
};
