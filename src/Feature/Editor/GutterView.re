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
      ~context: Draw.context,
      lineNumber: int,
      lineNumberWidth: float,
      theme: Theme.t,
      lineSetting,
      cursorLine: int,
      yOffset: float,
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
        *. context.charWidth
        /. 2.;

  Draw.text(
    ~context,
    ~x=lineNumberXOffset,
    ~y=yF,
    ~backgroundColor=theme.editorLineNumberBackground,
    ~color=lineNumberTextColor,
    lineNumber,
  );
};

let renderLineNumbers =
    (
      ~context,
      ~lineNumberWidth,
      ~height,
      ~theme: Theme.t,
      ~count,
      ~showLineNumbers,
      ~cursorLine,
    ) => {
  Draw.rect(
    ~context,
    ~x=0.,
    ~y=0.,
    ~width=lineNumberWidth,
    ~height=float(height),
    ~color=theme.editorLineNumberBackground,
  );

  Draw.renderImmediate(~context, ~count, (item, offset) =>
    renderLineNumber(
      ~context,
      item,
      lineNumberWidth,
      theme,
      showLineNumbers,
      cursorLine,
      offset,
    )
  );
};

let render =
    (
      ~showLineNumbers,
      ~lineNumberWidth,
      ~width,
      ~height,
      ~theme,
      ~editorFont: EditorFont.t,
      ~scrollY,
      ~lineHeight,
      ~count,
      ~cursorLine,
      ~diffMarkers,
      transform,
      _ctx,
    ) => {
  let context =
    Draw.{
      transform,
      width,
      height,
      scrollX: 0.,
      scrollY,
      lineHeight,
      fontFamily: editorFont.fontFile,
      fontSize: editorFont.fontSize,
      charWidth: editorFont.measuredWidth,
      charHeight: editorFont.measuredHeight,
    };

  if (showLineNumbers != LineNumber.Off) {
    renderLineNumbers(
      ~context,
      ~lineNumberWidth,
      ~height,
      ~theme,
      ~count,
      ~showLineNumbers,
      ~cursorLine,
    );
  };

  Option.iter(
    EditorDiffMarkers.render(
      ~scrollY=context.scrollY,
      ~rowHeight=context.lineHeight,
      ~x=lineNumberWidth,
      ~height=float(height),
      ~width=Constants.diffMarkerWidth,
      ~count,
      ~transform=context.transform,
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
      ~lineHeight,
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
      ~width=int_of_float(totalWidth),
      ~height,
      ~theme,
      ~editorFont,
      ~scrollY,
      ~lineHeight,
      ~count,
      ~cursorLine,
      ~diffMarkers,
    );

  (totalWidth, <OpenGL style render />);
};
