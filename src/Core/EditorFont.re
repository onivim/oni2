module Zed_utf8 = ZedBundled;

type t = {
  fontFile: string,
  boldFontFile: string,
  italicFontFile: string,
  boldItalicFontFile: string,
  fontSize: int,
  measuredWidth: float,
  measuredHeight: float,
};

let getHeight = (v: t) => v.measuredHeight;

let measure = (~text, v: t) => {
  float_of_int(Zed_utf8.length(text)) *. v.measuredWidth;
};

let create = (~fontFile, 
  ~boldFontFile,
  ~italicFontFile,
  ~boldItalicFontFile,
  ~fontSize, 
  ~measuredWidth, 
  ~measuredHeight, ()) => {
  boldFontFile,
  italicFontFile,
  boldItalicFontFile,
  fontFile,
  fontSize,
  measuredWidth,
  measuredHeight,
};
