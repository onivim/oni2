type t = {
  index: int,
  backgroundColor: Revery.Color.t,
  foregroundColor: Revery.Color.t,
  bold: bool,
  italic: bool,
};

let create = (~index, ~backgroundColor, ~foregroundColor, ~bold, ~italic, ()) => {
  index,
  backgroundColor,
  foregroundColor,
  bold,
  italic,
};
