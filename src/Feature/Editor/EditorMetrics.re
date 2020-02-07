/*
  * EditorMetrics.re
  *
  * Dimensions and measurements relating to an editor
 */

[@deriving show]
type t = {
  pixelWidth: int,
  pixelHeight: int,
  lineHeight: float,
  characterWidth: float,
};

let create = () => {
  {pixelWidth: 1000, pixelHeight: 1000, lineHeight: 1., characterWidth: 1.};
};
