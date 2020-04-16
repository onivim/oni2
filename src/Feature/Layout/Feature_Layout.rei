module WindowTree: {
  [@deriving show]
  type direction =
    | Horizontal
    | Vertical;

  type position =
    | Before
    | After;

  [@deriving show]
  type split = {
    id: int,
    editorGroupId: int,
    width: option(int),
    height: option(int),
  };

  type t;

  let getSplits: t => list(split);
  let createSplit:
    (~width: int=?, ~height: int=?, ~editorGroupId: int, unit) => split;
  let addSplit:
    (~target: option(int)=?, ~position: position, direction, split, t) => t;
  let removeSplit: (int, t) => t;
  let getEditorGroupIdFromSplitId: (int, t) => option(int);
  let rotateForward: (int, t) => t;
  let rotateBackward: (int, t) => t;
};

module WindowTreeLayout: {
  type t = {
    split: WindowTree.split,
    x: int,
    y: int,
    width: int,
    height: int,
  };

  let layout: (int, int, int, int, WindowTree.t) => list(t);
};

type direction =
  | Up
  | Left
  | Down
  | Right;

type t = {
  windowTree: WindowTree.t,
  activeWindowId: int,
  windowTreeWidth: int,
  windowTreeHeight: int,
};

let create: unit => t;

let setTreeSize: (int, int, t) => t;
let ensureActive: t => t;

let move: (direction, t) => int;
let moveLeft: t => int;
let moveRight: t => int;
let moveUp: t => int;
let moveDown: t => int;
