[@deriving show]
type t = {
  scancode: int,
  keycode: int,
  modifiers: Modifiers.t,
};
