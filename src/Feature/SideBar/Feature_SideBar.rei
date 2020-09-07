type pane =
  | FileExplorer
  | SCM
  | Extensions
  | Search;

type location =
  | Left
  | Right;

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
let location: model => location;

let isVisible: (pane, model) => bool;
let toggle: (pane, model) => model;
let setDefaultVisibility: (model, bool) => model;
let setDefaultLocation: (model, string) => model;

type settings = {
  sideBarLocation: string,
  sideBarVisibility: bool,
};

let setDefaults: (model, settings) => model;
