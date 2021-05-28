open EditorCoreTypes;
open Revery.UI;
open Oni_Core;

let renderLineNumber =
    (
      ~context: Draw.context,
      lineNumber: int,
      lineNumberWidth: float,
      colors: Colors.t,
      lineSetting,
      cursorLine: int,
    ) => {
  let isFirstBufferLine =
    Editor.viewLineIsPrimary(lineNumber, context.editor);

  if (isFirstBufferLine) {
    let bufferLine = Editor.viewLineToBufferLine(lineNumber, context.editor);
    let font =
      Service_Font.resolveWithFallback(
        ~italic=false,
        Revery.Font.Weight.Normal,
        context.fontFamily,
      );
    let fontMetrics = Revery.Font.getMetrics(font, context.fontSize);
    let isActiveLine =
      EditorCoreTypes.LineNumber.toZeroBased(bufferLine) == cursorLine;
    let ({y: yOffset, _}: PixelPosition.t, _) =
      Editor.bufferBytePositionToPixel(
        ~position=BytePosition.{line: bufferLine, byte: ByteIndex.zero},
        context.editor,
      );

    let paddingY = context.editor |> Editor.linePaddingInPixels;
    let y = paddingY +. yOffset -. fontMetrics.ascent;

    let lineNumber =
      string_of_int(
        LineNumber.getLineNumber(
          ~bufferLine=EditorCoreTypes.LineNumber.toOneBased(bufferLine),
          ~cursorLine=cursorLine + 1,
          ~setting=lineSetting,
          (),
        ),
      );

    let lineNumberXOffset =
      isActiveLine && lineSetting == `Relative
        ? 0.
        : lineNumberWidth
          /. 2.
          -. float(String.length(lineNumber))
          *. context.charWidth
          /. 2.;

    let color =
      isActiveLine
        ? colors.lineNumberActiveForeground : colors.lineNumberForeground;

    Draw.utf8Text(
      ~context,
      ~x=lineNumberXOffset,
      ~y,
      ~color,
      ~bold=false,
      ~italic=false,
      ~text=lineNumber,
    );
  };
};

let renderLineNumbers =
    (
      ~context,
      ~lineNumberWidth,
      ~height,
      ~colors: Colors.t,
      ~showLineNumbers,
      ~cursorLine,
    ) => {
  Draw.rect(
    ~context,
    ~x=0.,
    ~y=0.,
    ~width=lineNumberWidth,
    ~height=float(height),
    ~color=colors.gutterBackground,
  );

  Draw.renderImmediate(~context, (item, _) =>
    renderLineNumber(
      ~context,
      item,
      lineNumberWidth,
      colors,
      showLineNumbers,
      cursorLine,
    )
  );
};

let render =
    (
      ~editor,
      ~showLineNumbers,
      ~showScrollShadow,
      ~lineNumberWidth,
      ~width,
      ~height,
      ~colors,
      ~editorFont: Service_Font.font,
      ~cursorLine,
      ~diffMarkers,
      canvasContext,
      _,
    ) => {
  let context =
    Draw.createContext(~canvasContext, ~width, ~height, ~editor, ~editorFont);

  if (showLineNumbers != `Off) {
    renderLineNumbers(
      ~context,
      ~lineNumberWidth,
      ~height,
      ~colors,
      ~showLineNumbers,
      ~cursorLine,
    );
  };

  Option.iter(
    EditorDiffMarkers.render(
      ~context,
      ~rowHeight=Editor.lineHeightInPixels(editor),
      ~x=lineNumberWidth,
      ~width=Constants.diffMarkerWidth,
      ~canvasContext,
      ~colors,
    ),
    diffMarkers,
  );

  if (showScrollShadow) {
    ScrollShadow.renderVertical(
      ~color=colors.shadow,
      ~editor,
      ~width=float(width),
      ~context,
    );
  };
};

let make =
    (
      ~editor,
      ~showScrollShadow,
      ~showLineNumbers,
      ~height,
      ~colors,
      ~editorFont: Service_Font.font,
      ~count,
      ~cursorLine,
      ~diffMarkers,
      (),
    ) => {
  let lineNumberWidth =
    showLineNumbers != `Off
      ? LineNumber.getLineNumberPixelWidth(
          ~lines=count,
          ~fontPixelWidth=editorFont.underscoreWidth,
          (),
        )
      : 0.0;

  let totalWidth = Editor.gutterWidth(~editorFont, editor);

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
      ~editor,
      ~showScrollShadow,
      ~showLineNumbers,
      ~lineNumberWidth,
      ~width=int_of_float(totalWidth),
      ~height,
      ~colors,
      ~editorFont,
      ~cursorLine,
      ~diffMarkers,
    );

  (totalWidth, <Canvas style render />);
};
