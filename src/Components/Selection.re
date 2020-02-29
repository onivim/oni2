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

let withinLength = (text: string, position: int): int => {
  let withinStart = max(position, 0);
  let withinBoth = min(withinStart, String.length(text));

  withinBoth;
};

let create = (~text: string, ~anchor: int, ~focus: int): t => {
  let safeAnchor = withinLength(text, anchor);
  let safeFocus = withinLength(text, focus);

  {anchor: safeAnchor, focus: safeFocus};
};

let rangeWidth = (selection: t): int => {
  abs(selection.focus - selection.anchor);
};

let rangeStart = (selection: t): int => {
  min(selection.focus, selection.anchor);
};

let rangeEnd = (selection: t): int => {
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
