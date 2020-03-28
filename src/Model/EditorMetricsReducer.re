/*
  * EditorMetricsReducer.re
 */

open Actions;
open Feature_Editor;

let reduce = (v: EditorMetrics.t, action) => {
  switch (action) {
  | EditorGroupSetSize(_, {pixelWidth, pixelHeight}) =>
    EditorMetrics.{pixelWidth, pixelHeight}
  | _ => v
  };
};
