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
      ~name="indentation.detectAndSet", _dispatch => {
      open State;
      let buffer = Buffers.getBuffer(bufferId, state.buffers);
      switch (buffer) {
      | None => ()
      | Some(b) =>
        if (!Buffer.isIndentationSet(b)) {
          print_endline("Indentation not set! Detecting");

          let f = Buffer.getLine(b);
          let count = Buffer.getNumberOfLines(b);

          print_endline(
            "Number of lines checking: " ++ string_of_int(count),
          );

          let guess = IndentationGuesser.guessIndentation(~f, count, 4, true);

          let modeStr =
            switch (guess.mode) {
            | Tabs => "tabs"
            | Spaces => "spaces"
            };

          print_endline(
            "Guessed indentation: "
            ++ string_of_int(guess.size)
            ++ " | "
            ++ modeStr,
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
