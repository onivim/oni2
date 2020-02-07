module Zed_utf8 = ZedBundled;

type t = {
  fontFile: string,
  fontSize: float,
  measuredWidth: float,
  measuredHeight: float,
  descenderHeight: float,
};

let getHeight = (v: t) => v.measuredHeight;

let measure = (~text, v: t) => {
  float_of_int(Zed_utf8.length(text)) *. v.measuredWidth;
};

let create =
    (
      ~fontFile,
      ~fontSize,
      ~measuredWidth,
      ~descenderHeight,
      ~measuredHeight,
      (),
    ) => {
  fontFile,
  fontSize,
  measuredWidth,
  measuredHeight,
  descenderHeight,
};

let toString = editorFont => {
  Printf.sprintf(
    "fontFile: %s\n fontSize: %f\n measuredWidth: %f\n measuredHeight: %f\n",
    editorFont.fontFile,
    editorFont.fontSize,
    editorFont.measuredWidth,
    editorFont.measuredHeight,
  );
};
