open Utility;

[@deriving show({with_path: false})]
type t = {
  anchor: int,
  focus: int,
};

let initial: t = {anchor: 0, focus: 0};

let anchor = (selection: t): int => {
  selection.anchor;
};

let focus = (selection: t): int => {
  selection.focus;
};

let create = (~text: string, ~anchor: int, ~focus: int): t => {
  let safeOffset = IntEx.clamp(~lo=0, ~hi=String.length(text));

  let safeAnchor = safeOffset(anchor);
  let safeFocus = safeOffset(focus);

  {anchor: safeAnchor, focus: safeFocus};
};

let length = (selection: t): int => {
  abs(selection.focus - selection.anchor);
};

let offsetLeft = (selection: t): int => {
  min(selection.focus, selection.anchor);
};

let offsetRight = (selection: t): int => {
  max(selection.focus, selection.anchor);
};

let isCollapsed = (selection: t): bool => {
  selection.anchor == selection.focus;
};

let collapsed = (~text: string, offset: int): t => {
  create(~text, ~anchor=offset, ~focus=offset);
};

let extend = (~text: string, ~selection: t, offset: int): t => {
  create(~text, ~anchor=selection.anchor, ~focus=offset);
};
