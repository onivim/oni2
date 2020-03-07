/*
  * EditorMetricsReducer.re
 */

open Actions;
open Feature_Editor;

let reduce = (v: EditorMetrics.t, action) => {
  switch (action) {
  | EditorGroupSetSize(_, {pixelWidth, pixelHeight}) => {
      ...v,
      pixelWidth,
      pixelHeight,
    }
  | EditorFont(Service_Font.FontLoaded({measuredHeight, measuredWidth, _})) => {
      ...v,
      lineHeight: measuredHeight,
      characterWidth: measuredWidth,
    }
  | _ => v
  };
};
