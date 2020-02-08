type pane =
  | FileExplorer
  | SCM
  | Extensions;

type t = {
  isOpen: bool,
  selected: pane,
};

let initial: t;

let isVisible: (pane, t) => bool;
let toggle: (pane, t) => t;
