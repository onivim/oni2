/*
 * IndentationSettings.re
 */

type mode =
  | Tabs
  | Spaces;

type t = {
  mode,
  size: int,
  tabSize: int,
};

let default = {mode: Spaces, size: 2, tabSize: 2};

let ofConfiguration = (configuration: Configuration.t) => {
  mode: configuration.editorInsertSpaces ? Spaces : Tabs,
  size: configuration.editorTabSize,
  tabSize: configuration.editorTabSize,
};

let create = (~mode, ~size, ~tabSize, ()) => {mode, size, tabSize};
