/*
  * EditorMetricsReducer.re
 */

open Actions;
open Feature_Editor;

let reduce = (v: EditorMetrics.t, action) => {
  switch (action) {
  | EditorGroupSizeChanged({width, height, _}) =>
    EditorMetrics.{pixelWidth: width, pixelHeight: height}
  | _ => v
  };
};
