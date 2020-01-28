module Zed_utf8 = ZedBundled;

type t = {
  fontFile: string,
  fontSize: float,
  measuredWidth: float,
  measuredHeight: float,
};

let getHeight = (v: t) => v.measuredHeight;

let measure = (~text, v: t) => {
  float_of_int(Zed_utf8.length(text)) *. v.measuredWidth;
};

let create = (~fontFile, ~fontSize, ~measuredWidth, ~measuredHeight, ()) => {
  fontFile,
  fontSize,
  measuredWidth,
  measuredHeight,
};
