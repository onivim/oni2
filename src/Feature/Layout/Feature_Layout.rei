module WindowTree: {
  // definition only used for tests
  type t =
    | Split([ | `Horizontal | `Vertical], list(t))
    | Window({
        weight: float,
        content: int,
      })
    | Empty;

  let empty: t; // only used for tests

  let getSplits: t => list(int);
  let addSplit:
    (
      ~target: option(int)=?,
      ~position: [ | `Before | `After],
      [ | `Horizontal | `Vertical],
      int,
      t
    ) =>
    t;
  let removeSplit: (int, t) => t;

  // only used for tests
  let rotateForward: (int, t) => t;
  let rotateBackward: (int, t) => t;
};

module WindowTreeLayout: {
  type t = {
    content: int,
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

type t = {windowTree: WindowTree.t};

let create: unit => t;

let move: (direction, int, t) => int;
let moveLeft: (int, t) => int;
let moveRight: (int, t) => int;
let moveUp: (int, t) => int;
let moveDown: (int, t) => int;

let rotateForward: (int, t) => t;
let rotateBackward: (int, t) => t;
