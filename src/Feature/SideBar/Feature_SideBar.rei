type pane =
  | FileExplorer
  | SCM
  | Extensions;

type position =
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
let position: model => position;

let isVisible: (pane, model) => bool;
let toggle: (pane, model) => model;
let setDefaultVisibility: (model, bool) => model;
let setDefaultPosition: (model, string) => model;

type settings = {
  sideBarPosition: string,
  sideBarVisibility: bool,
};

let setDefaults: (model, settings) => model;
