type direction =
  | Up
  | Left
  | Down
  | Right;

// definition only used for tests
type t =
  | Split([ | `Horizontal | `Vertical], list(t))
  | Window({
      weight: float,
      content: int,
    })
  | Empty;

[@deriving show]
type window = {
  content: int,
  x: int,
  y: int,
  width: int,
  height: int,
};

module Internal: {
  let move: (int, int, int, list(window)) => option(int); // only used for tests
};

let initial: t;

let windows: t => list(int);
let addWindow:
  (
    ~target: option(int)=?,
    ~position: [ | `Before | `After],
    [ | `Horizontal | `Vertical],
    int,
    t
  ) =>
  t;
let removeWindow: (int, t) => t;

let layout: (int, int, int, int, t) => list(window);

let move: (direction, int, t) => int;
let moveLeft: (int, t) => int;
let moveRight: (int, t) => int;
let moveUp: (int, t) => int;
let moveDown: (int, t) => int;

let rotateForward: (int, t) => t;
let rotateBackward: (int, t) => t;
