type pane =
  | FileExplorer
  | SCM
  | Extensions;

type model;

let initial: model;

let isOpen: model => bool;
let selected: model => pane;

let isVisible: (pane, model) => bool;
let toggle: (pane, model) => model;
let setDefaultVisibility: (model, bool) => model;
