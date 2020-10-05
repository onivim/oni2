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

let default = {mode: Spaces, size: 4, tabSize: 4};

let create = (~mode, ~size, ~tabSize, ()) => {mode, size, tabSize};

let isEqual = (a: t, b: t) => {
  a.mode == b.mode && a.size == b.size && a.tabSize == b.tabSize;
};
