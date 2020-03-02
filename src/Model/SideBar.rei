type pane =
  | FileExplorer
  | SCM
  | Extensions;

type t = {
  openByDefault: bool,
  isOpen: bool,
  selected: pane,
};

let initial: t;

let isVisible: (pane, t) => bool;
let toggle: (pane, t) => t;
let setDefaultVisibility: (t, bool) => t;
