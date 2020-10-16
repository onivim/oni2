[@deriving show]
type direction =
  | CursorCenterVertically // zz
  | CursorCenterHorizontally
  | CursorTop // zt
  | CursorBottom // zb
  | CursorLeft
  | CursorRight
  | LineUp
  | LineDown
  | HalfPageUp
  | HalfPageDown
  | PageDown
  | PageUp;
