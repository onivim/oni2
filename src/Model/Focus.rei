[@deriving show]
type focusable =
  | Editor // TODO: Carry editor/buffer id
  | Wildmenu
  | Quickmenu
  | Search
  | FileExplorer;

type stack;

let initial: stack;

let push: (focusable, stack) => stack;
let pop: (focusable, stack) => stack;

let current: stack => option(focusable);
