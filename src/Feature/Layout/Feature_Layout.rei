type direction =
  | Up
  | Left
  | Down
  | Right;

// definition only used for tests
[@deriving show({with_path: false})]
type size =
  | Weight(float);

// definition only used for tests
[@deriving show({with_path: false})]
type t('id) =
  | Split([ | `Horizontal | `Vertical], size, list(t('id)))
  | Window(size, 'id);

[@deriving show]
type sizedWindow('id) = {
  id: 'id,
  x: int,
  y: int,
  width: int,
  height: int,
};

module Internal: {
  let move: ('id, int, int, list(sizedWindow('id))) => option('id); // only used for tests
};

let initial: t('id);

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

let layout: (int, int, int, int, t('id)) => list(sizedWindow('id));

let move: (direction, 'id, t('id)) => 'id;
let moveLeft: ('id, t('id)) => 'id;
let moveRight: ('id, t('id)) => 'id;
let moveUp: ('id, t('id)) => 'id;
let moveDown: ('id, t('id)) => 'id;

let rotateForward: ('id, t('id)) => t('id);
let rotateBackward: ('id, t('id)) => t('id);

let resizeWindow:
  ([ | `Horizontal | `Vertical], 'id, float, t('id)) => t('id);
