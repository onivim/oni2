/*
 * IndentationSettings.re
 */

[@deriving show]
type mode =
  | Tabs
  | Spaces;

[@deriving show]
type t = {
  mode,
  size: int,
  tabSize: int,
};

let default = {mode: Spaces, size: 4, tabSize: 4};

let create = (~mode, ~size, ~tabSize, ()) => {mode, size, tabSize};

let isEqual = (a: t, b: t) => {
  a.mode == b.mode && a.size == b.size && a.tabSize == b.tabSize;
};
