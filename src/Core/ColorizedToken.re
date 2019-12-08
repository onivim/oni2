type t = {
  index: int,
  backgroundColor: Revery.Color.t,
  foregroundColor: Revery.Color.t,
};

let create = (~index, ~backgroundColor, ~foregroundColor, ()) => {
  index,
  backgroundColor,
  foregroundColor,
};
