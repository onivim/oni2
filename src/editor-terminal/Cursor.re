type t = {
  row: int,
  column: int,
  visible: bool,
  shape: Vterm.TermProp.CursorShape.t,
};

let initial = {
  row: 0,
  column: 0,
  visible: false,
  shape: Vterm.TermProp.CursorShape.Block,
};
