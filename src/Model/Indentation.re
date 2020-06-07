/*
 * Indentation.re
 */

open Oni_Core;

let getForActiveBuffer = (state: State.t) => {
  let activeBuffer = Selectors.getActiveBuffer(state);
  switch (activeBuffer) {
  | None => IndentationSettings.ofConfiguration(state.configuration)
  | Some(b) =>
    let bufferIndentation = Buffer.getIndentation(b);
    switch (bufferIndentation) {
    | None => IndentationSettings.ofConfiguration(state.configuration)
    | Some(i) => i
    };
  };
};
