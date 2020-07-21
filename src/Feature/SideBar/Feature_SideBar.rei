type pane =
  | FileExplorer
  | SCM
  | Extensions;

type model;

[@deriving show]
type msg =
  | ResizeInProgress(int)
  | ResizeCommitted;

let update: (msg, model) => model;

let initial: model;

let width: model => int;
let isOpen: model => bool;
let selected: model => pane;

let isVisible: (pane, model) => bool;
let toggle: (pane, model) => model;
let setDefaultVisibility: (model, bool) => model;
