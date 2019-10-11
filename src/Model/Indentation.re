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

let toStatusString = (indentation: IndentationSettings.t) => {
  switch (indentation.mode) {
  | Tabs => "Tabs: " ++ string_of_int(indentation.tabSize)
  | Spaces => "Spaces: " ++ string_of_int(indentation.size)
  };
};
