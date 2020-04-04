/*
  * EditorMetrics.re
  *
  * Dimensions and measurements relating to an editor
 */

[@deriving show]
type t = {
  pixelWidth: int,
  pixelHeight: int,
};

let create = () => {
  {pixelWidth: 1000, pixelHeight: 1000};
};
