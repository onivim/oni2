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
    editorGroupId: int,
  };

  // definition only used for tests
  type t =
    | Parent(direction, list(t))
    | Leaf(split)
    | Empty;

  let empty: t; // only used for tests

  let getSplits: t => list(split);
  let createSplit:
    (~editorGroupId: int, unit) => split;
  let addSplit:
    (~target: option(int)=?, ~position: position, direction, split, t) => t;
  let removeSplit: (int, t) => t;

  // only used for tests
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
  let move: (int, int, int, list(t)) => option(int); // only used for tests
};

type direction =
  | Up
  | Left
  | Down
  | Right;

type t = {
  windowTree: WindowTree.t,
  windowTreeWidth: int,
  windowTreeHeight: int,
};

let create: unit => t;

let setTreeSize: (int, int, t) => t;

let move: (direction, int, t) => int;
let moveLeft: (int, t) => int;
let moveRight: (int, t) => int;
let moveUp: (int, t) => int;
let moveDown: (int, t) => int;

let rotateForward: (int, t) => t;
let rotateBackward: (int, t) => t;
