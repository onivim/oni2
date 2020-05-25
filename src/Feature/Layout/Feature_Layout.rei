open Oni_Core;

// MODEL

// definition only used for tests
//[@deriving show({with_path: false})]
//type size =
//  | Weight(float);
//
// definition only used for tests
//[@deriving show({with_path: false})]
//type t('id) =
//  | Split([ | `Horizontal | `Vertical], size, list(t('id)))
//  | Window(size, 'id);
//
//[@deriving show]
//type sized('id) = {
//  x: int,
//  y: int,
//  width: int,
//  height: int,
//  kind: [
//    | `Split([ | `Horizontal | `Vertical], list(sized('id)))
//    | `Window('id)
//  ],
//};
type t('id);

//
//module Internal: {
//  let move: ('id, int, int, sized('id)) => option('id); // only used for tests
//};
//
let initial: t('id);
//
let windows: t('id) => list('id);
let addWindow:
  (
    ~target: option('id)=?,
    ~position: [ | `Before | `After],
    [ | `Horizontal | `Vertical],
    'id,
    t('id)
  ) =>
  t('id);
let removeWindow: ('id, t('id)) => t('id);

//let layout: (int, int, int, int, t('id)) => sized('id);

let move: ('id, int, int, t('id)) => 'id;
let moveLeft: ('id, t('id)) => 'id;
let moveRight: ('id, t('id)) => 'id;
let moveUp: ('id, t('id)) => 'id;
let moveDown: ('id, t('id)) => 'id;

let rotateForward: ('id, t('id)) => t('id);
let rotateBackward: ('id, t('id)) => t('id);

let resizeWindow:
  ([ | `Horizontal | `Vertical], 'id, float, t('id)) => t('id);
let resizeSplit: (~path: list(int), ~delta: float, t('id)) => t('id);
let resetWeights: t('id) => t('id);

// UPDATE

type command =
  | MoveLeft
  | MoveRight
  | MoveUp
  | MoveDown
  | RotateForward
  | RotateBackward
  | DecreaseSize
  | IncreaseSize
  | DecreaseHorizontalSize
  | IncreaseHorizontalSize
  | DecreaseVerticalSize
  | IncreaseVerticalSize
  | ResetSizes;

[@deriving show]
type msg =
  | HandleDragged({
      path: list(int),
      delta: float,
    })
  | Command(command);

type outmsg('id) =
  | Nothing
  | Focus('id);

let update: (~focus: option('id), t('id), msg) => (t('id), outmsg('id));

// VIEW

module View: {
  open Revery.UI;

  let make:
    (
      ~children: 'id => element,
      ~model: t('id),
      ~theme: ColorTheme.Colors.t,
      ~dispatch: msg => unit,
      unit
    ) =>
    element;
};

// CONTRIBUTIONS

module Contributions: {let commands: list(Command.t(msg));};
