[@deriving show]
type t = {
  start: PixelPosition.t,
  stop: PixelPosition.t,
};

let zero = {start: PixelPosition.zero, stop: PixelPosition.zero};

let create = (~start, ~stop) => {start, stop};
