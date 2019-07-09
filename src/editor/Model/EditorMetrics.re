/*
  * EditorMetrics.re
  *
  * Dimensions and measurements relating to an editor
 */

open Actions;

type t = Actions.editorMetrics;

let create = () => {
  {pixelWidth: 1000, pixelHeight: 1000, lineHeight: 1., characterWidth: 1.};
};
