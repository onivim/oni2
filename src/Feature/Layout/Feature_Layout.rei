type direction =
  | Up
  | Left
  | Down
  | Right;

// definition only used for tests
type t('id) =
  | Split([ | `Horizontal | `Vertical], list(container('id)))
  | Window('id)
  | Empty
and container('id) = {
  weight: float,
  content: t('id),
};

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

let rotateForward: ('content, t('content)) => t('content);
let rotateBackward: ('content, t('content)) => t('content);
