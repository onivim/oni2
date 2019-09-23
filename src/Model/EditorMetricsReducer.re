/*
  * EditorMetricsReducer.re
 */

open Actions;

let reduce = (v: EditorMetrics.t, action) => {
  switch (action) {
  | EditorGroupSetSize(_, {pixelWidth, pixelHeight}) => {
      ...v,
      pixelWidth,
      pixelHeight,
    }
  | SetEditorFont({measuredHeight, measuredWidth, _}) => {
      ...v,
      lineHeight: measuredHeight,
      characterWidth: measuredWidth,
    }
  | _ => v
  };
};
