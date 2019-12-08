[@deriving show({with_path: false})]
type t = {
  pixelWidth: int,
  pixelHeight: int,
};

let create = (~pixelWidth: int, ~pixelHeight: int, ()) => {
  pixelWidth,
  pixelHeight,
};