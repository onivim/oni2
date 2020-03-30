/*
  * EditorMetricsReducer.re
 */

open Actions;
open Feature_Editor;

let reduce = (v: EditorMetrics.t, action) => {
  switch (action) {
  | EditorGroupSetSize({width, height, _}) =>
    EditorMetrics.{pixelWidth: width, pixelHeight: height}
  | _ => v
  };
};
