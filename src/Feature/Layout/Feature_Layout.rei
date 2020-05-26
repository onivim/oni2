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
type model;

//
//module Internal: {
//  let move: ('id, int, int, sized('id)) => option('id); // only used for tests
//};
//
let initial: int => model;

let windows: model => list(int);
let addWindow: ([ | `Horizontal | `Vertical], int, model) => model;
let insertWindow:
  (
    [ | `Before(int) | `After(int)],
    [ | `Horizontal | `Vertical],
    int,
    model
  ) =>
  model;
let removeWindow: (int, model) => model;

//let layout: (int, int, int, int, t('id)) => sized('id);

// let move: ('id, int, int, t('id)) => 'id;
// let moveLeft: ('id, t('id)) => 'id;
// let moveRight: ('id, t('id)) => 'id;
// let moveUp: ('id, t('id)) => 'id;
// let moveDown: ('id, t('id)) => 'id;

// let rotateForward: ('id, t('id)) => t('id);
// let rotateBackward: ('id, t('id)) => t('id);

// let resizeWindow:
//   ([ | `Horizontal | `Vertical], 'id, float, t('id)) => t('id);
// let resizeSplit: (~path: list(int), ~delta: float, t('id)) => t('id);
// let resetWeights: t('id) => t('id);

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

type outmsg =
  | Nothing
  | Focus(int);

let update: (~focus: option(int), model, msg) => (model, outmsg);

// VIEW

module View: {
  open Revery.UI;

  let make:
    (
      ~children: int => element,
      ~model: model,
      ~theme: ColorTheme.Colors.t,
      ~dispatch: msg => unit,
      unit
    ) =>
    element;
};

// CONTRIBUTIONS

module Contributions: {let commands: list(Command.t(msg));};
