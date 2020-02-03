open Revery.Draw;
open Revery.UI;
open Oni_Core;

module Option = Utility.Option;

module Constants = {
  include Constants;

  let diffMarkersMaxLineCount = 2000;
  let diffMarkerWidth = 3.;
  let gutterMargin = 3.;
};

let renderLineNumber =
    (
      lineNumber: int,
      lineNumberWidth: float,
      theme: Theme.t,
      editorFont: EditorFont.t,
      lineSetting,
      cursorLine: int,
      yOffset: float,
      transform,
    ) => {
  let isActiveLine = lineNumber == cursorLine;
  let lineNumberTextColor =
    isActiveLine
      ? theme.editorActiveLineNumberForeground
      : theme.editorLineNumberForeground;

  let yF = yOffset;

  let lineNumber =
    string_of_int(
      LineNumber.getLineNumber(
        ~bufferLine=lineNumber + 1,
        ~cursorLine=cursorLine + 1,
        ~setting=lineSetting,
        (),
      ),
    );

  let lineNumberXOffset =
    isActiveLine
      ? 0.
      : lineNumberWidth
        /. 2.
        -. float(String.length(lineNumber))
        *. editorFont.measuredWidth
        /. 2.;

  Revery.Draw.Text.drawString(
    ~window=Revery.UI.getActiveWindow(),
    ~transform,
    ~x=lineNumberXOffset,
    ~y=yF,
    ~backgroundColor=theme.editorLineNumberBackground,
    ~color=lineNumberTextColor,
    ~fontFamily=editorFont.fontFile,
    ~fontSize=editorFont.fontSize,
    lineNumber,
  );
};

let renderLineNumbers =
    (
      ~transform,
      ~lineNumberWidth,
      ~height,
      ~theme: Theme.t,
      ~editorFont,
      ~scrollY,
      ~rowHeight,
      ~count,
      ~showLineNumbers,
      ~cursorLine,
    ) => {
  Shapes.drawRect(
    ~transform,
    ~x=0.,
    ~y=0.,
    ~width=lineNumberWidth,
    ~height=float(height),
    ~color=theme.editorLineNumberBackground,
    (),
  );

  ImmediateList.render(
    ~scrollY,
    ~rowHeight,
    ~height=float(height),
    ~count,
    ~render=
      (item, offset) => {
        renderLineNumber(
          item,
          lineNumberWidth,
          theme,
          editorFont,
          showLineNumbers,
          cursorLine,
          offset,
          transform,
        )
      },
    (),
  );
};

let render =
    (
      ~showLineNumbers,
      ~lineNumberWidth,
      ~height,
      ~theme,
      ~editorFont,
      ~scrollY,
      ~rowHeight,
      ~count,
      ~cursorLine,
      ~diffMarkers,
      transform,
      _ctx,
    ) => {
  if (showLineNumbers != LineNumber.Off) {
    renderLineNumbers(
      ~transform,
      ~lineNumberWidth,
      ~height,
      ~theme,
      ~editorFont,
      ~scrollY,
      ~rowHeight,
      ~count,
      ~showLineNumbers,
      ~cursorLine,
    );
  };

  Option.iter(
    EditorDiffMarkers.render(
      ~scrollY,
      ~rowHeight,
      ~x=lineNumberWidth,
      ~height=float(height),
      ~width=Constants.diffMarkerWidth,
      ~count,
      ~transform,
      ~theme,
    ),
    diffMarkers,
  );
};

let make =
    (
      ~showLineNumbers,
      ~height,
      ~theme,
      ~editorFont: EditorFont.t,
      ~scrollY,
      ~rowHeight,
      ~count,
      ~cursorLine,
      ~diffMarkers,
      (),
    ) => {
  let lineNumberWidth =
    showLineNumbers != LineNumber.Off
      ? LineNumber.getLineNumberPixelWidth(
          ~lines=count,
          ~fontPixelWidth=editorFont.measuredWidth,
          (),
        )
      : 0.0;

  let totalWidth =
    lineNumberWidth +. Constants.diffMarkerWidth +. Constants.gutterMargin;

  let style =
    Style.[
      overflow(`Hidden),
      position(`Absolute),
      top(0),
      left(0),
      width(int_of_float(totalWidth)),
      bottom(0),
    ];

  let render =
    render(
      ~showLineNumbers,
      ~lineNumberWidth,
      ~height,
      ~theme,
      ~editorFont,
      ~scrollY,
      ~rowHeight,
      ~count,
      ~cursorLine,
      ~diffMarkers,
    );

  (totalWidth, <OpenGL style render />);
};
