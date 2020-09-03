[@deriving show]
type t = {
  start: PixelPosition.t,
  stop: PixelPosition.t,
};

let zero: t;

let create: (~start: PixelPosition.t, ~stop: PixelPosition.t) => t;
