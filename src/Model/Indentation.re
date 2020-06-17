/*
 * Indentation.re
 */

open Oni_Core;

let getForActiveBuffer = (state: State.t) => {
  let activeBuffer = Selectors.getActiveBuffer(state);
  switch (activeBuffer) {
  | None => IndentationSettings.ofConfiguration(state.configuration)
  | Some(buffer) =>
    Oni_Core.Indentation.getForBuffer(~buffer, state.configuration)
  };
};
