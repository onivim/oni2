open Revery.UI;
open Oni_Core;

module Constants = {
  include Constants;

  let diffMarkersMaxLineCount = 2000;
  let diffMarkerWidth = 3.;
  let gutterMargin = 3.;
};

let lineNumberPaint = Skia.Paint.make();
Skia.Paint.setTextEncoding(lineNumberPaint, Utf8);
Skia.Paint.setAntiAlias(lineNumberPaint, true);
Skia.Paint.setLcdRenderText(lineNumberPaint, true);
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
  let y = yOffset -. context.fontMetrics.ascent;

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

  let lineNumberTextColor =
    isActiveLine
      ? theme.editorActiveLineNumberForeground
      : theme.editorLineNumberForeground;
  let paint = lineNumberPaint;
  Skia.Paint.setColor(paint, Revery.Color.toSkia(lineNumberTextColor));
  Skia.Paint.setTextSize(paint, context.fontSize);
  Skia.Paint.setTypeface(paint, Revery.Font.getSkiaTypeface(context.font));

  Draw.text(~context, ~x=lineNumberXOffset, ~y, ~paint, lineNumber);
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
  let paint = Skia.Paint.make();
  Skia.Paint.setColor(
    paint,
    Revery.Color.toSkia(theme.editorLineNumberBackground),
  );

  Draw.rect(
    ~context,
    ~x=0.,
    ~y=0.,
    ~width=lineNumberWidth,
    ~height=float(height),
    ~paint,
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
      canvasContext,
    ) => {
  let context =
    Draw.createContext(
      ~canvasContext,
      ~width,
      ~height,
      ~scrollX=0.,
      ~scrollY,
      ~lineHeight,
      ~editorFont,
    );

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
      ~canvasContext,
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

  (totalWidth, <Canvas style render />);
};
