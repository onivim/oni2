type t = Service_Font.t;

let getHeight = Service_Font.getHeight;

let measure = Service_Font.measure;

/*let create =
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
  };*/

let toString = editorFont => {
  Service_Font.(
    Printf.sprintf(
      "fontFile: %s\n fontSize: %f\n measuredWidth: %f\n measuredHeight: %f\n",
      editorFont.fontFile,
      editorFont.fontSize,
      editorFont.measuredWidth,
      editorFont.measuredHeight,
    )
  );
};
