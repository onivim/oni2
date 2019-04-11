/*
 * Indentation.re
 */

open Oni_Core;

let getForActiveBuffer = (state: State.t) => {
  IndentationSettings.ofConfiguration(state.configuration);
};

let toStatusString = (indentation: IndentationSettings.t) => {
  switch (indentation.mode) {
  | Tabs => "Tabs: " ++ string_of_int(indentation.tabSize)
  | Spaces => "Spaces: " ++ string_of_int(indentation.size)
  };
};
