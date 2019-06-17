/*
  * EditorMetrics.re
  *
  * Dimensions and measurements relating to an editor
 */

open Actions;

type t = {
  pixelWidth: int,
  pixelHeight: int,
  lineHeight: float,
  characterWidth: float,
};

let create = () => {
  {pixelWidth: 1000, pixelHeight: 1000, lineHeight: 1., characterWidth: 1.};
};

let toLinesAndColumns = (v: t) => {
    let numberOfLines = int_of_float(float_of_int(v.pixelHeight) /. v.lineHeight);
    let numberOfColumns = int_of_float(float_of_int(v.pixelWidth) /. v.characterWidth);
    (numberOfLines, numberOfColumns);
}

let reduce = (v: t, action) => {
  switch (action) {
  | SetEditorSize({pixelWidth, pixelHeight}) => {
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
