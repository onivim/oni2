type direction =
  | Up
  | Left
  | Down
  | Right;

// definition only used for tests
type t('content) =
  | Split([ | `Horizontal | `Vertical], list(t('content)))
  | Window({
      weight: float,
      content: 'content,
    })
  | Empty;

[@deriving show]
type window('content) = {
  content: 'content,
  x: int,
  y: int,
  width: int,
  height: int,
};

module Internal: {
  let move:
    ('content, int, int, list(window('content))) => option('content); // only used for tests
};

let initial: t('content);

let windows: t('content) => list('content);
let addWindow:
  (
    ~target: option('content)=?,
    ~position: [ | `Before | `After],
    [ | `Horizontal | `Vertical],
    'content,
    t('content)
  ) =>
  t('content);
let removeWindow: ('content, t('content)) => t('content);

let layout: (int, int, int, int, t('content)) => list(window('content));

let move: (direction, 'content, t('content)) => 'content;
let moveLeft: ('content, t('content)) => 'content;
let moveRight: ('content, t('content)) => 'content;
let moveUp: ('content, t('content)) => 'content;
let moveDown: ('content, t('content)) => 'content;

let rotateForward: ('content, t('content)) => t('content);
let rotateBackward: ('content, t('content)) => t('content);
