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
  | HalfPageDown
  | HalfPageUp
  | PageDown
  | PageUp
  | HalfPageLeft
  | HalfPageRight
  | ColumnLeft
  | ColumnRight;
