open EditorCoreTypes;
open Revery.Draw;
open Revery.UI;

open Oni_Core;

open Helpers;

module Log = (val Log.withNamespace("Oni2.Editor.SurfaceView"));

module Styles = {
  open Style;

  let bufferViewClipped = bufferPixelWidth => [
    overflow(`Hidden),
    position(`Absolute),
    top(0),
    left(0),
    width(int_of_float(bufferPixelWidth)),
    bottom(0),
  ];
};

let drawCurrentLineHighlight =
    (
      line,
      ~transform,
      ~gutterWidth,
      ~metrics: EditorMetrics.t,
      ~scrollY,
      ~lineHeight,
      ~theme: Theme.t,
    ) =>
  Shapes.drawRect(
    ~transform,
    ~x=gutterWidth,
    ~y=lineHeight *. float(Index.toZeroBased(line)) -. scrollY,
    ~height=lineHeight,
    ~width=float(metrics.pixelWidth) -. gutterWidth,
    ~color=theme.editorLineHighlightBackground,
    (),
  );

let drawRuler = (x, ~transform, ~metrics: EditorMetrics.t, ~theme: Theme.t) =>
  Shapes.drawRect(
    ~transform,
    ~x,
    ~y=0.0,
    ~height=float(metrics.pixelHeight),
    ~width=1.,
    ~color=theme.editorRulerForeground,
    (),
  );

let renderRulers =
    (
      ~rulers,
      ~gutterWidth,
      ~scrollX,
      ~scrollY,
      ~editorFont,
      ~transform,
      ~metrics,
      ~theme,
    ) =>
  rulers
  |> List.map(
       bufferPositionToPixel(
         ~gutterWidth,
         ~scrollX,
         ~scrollY,
         ~editorFont,
         0,
       ),
     )
  |> List.map(fst)
  |> List.iter(drawRuler(~transform, ~metrics, ~theme));

let make =
    (
      ~onScroll,
      ~elementRef,
      ~buffer,
      ~editor,
      ~metrics,
      ~gutterWidth,
      ~theme,
      ~showLineNumbers,
      ~topVisibleLine,
      ~onCursorChange,
      ~layout: EditorLayout.t,
      ~cursorPosition: Location.t,
      ~rulers,
      ~lineNumberWidth,
      ~editorFont: EditorFont.t,
      ~diffMarkers,
      ~leftVisibleColumn,
      ~diagnosticsMap,
      ~selectionRanges,
      ~matchingPairs,
      ~bufferHighlights,
      ~definition,
      ~bufferSyntaxHighlights,
      ~shouldRenderWhitespace,
      ~shouldRenderIndentGuides,
      ~bottomVisibleLine,
      ~shouldHighlightActiveIndentGuides,
      ~mode,
      ~isActiveSplit,
      (),
    ) => {
  let Editor.{scrollX, scrollY, _} = editor;
  let bufferPixelWidth =
    layout.lineNumberWidthInPixels +. layout.bufferWidthInPixels;
  let lineCount = Buffer.getNumberOfLines(buffer);
  let indentation =
    switch (Buffer.getIndentation(buffer)) {
    | Some(v) => v
    | None => IndentationSettings.default
    };

  let onMouseWheel = (wheelEvent: NodeEvents.mouseWheelEventParams) =>
    onScroll(wheelEvent.deltaY *. (-50.));

  let onMouseUp = (evt: NodeEvents.mouseButtonEventParams) => {
    Log.trace("editorMouseUp");

    switch (elementRef) {
    | None => ()
    | Some(r) =>
      let rect = r#getBoundingBox() |> Revery.Math.Rectangle.ofBoundingBox;

      let relY = evt.mouseY -. Revery.Math.Rectangle.getY(rect);
      let relX = evt.mouseX -. Revery.Math.Rectangle.getX(rect);

      let numberOfLines = Buffer.getNumberOfLines(buffer);
      let (line, col) =
        Editor.pixelPositionToLineColumn(
          editor,
          metrics,
          relX -. gutterWidth,
          relY,
        );

      if (line < numberOfLines) {
        Log.tracef(m => m("  topVisibleLine is %i", topVisibleLine));
        Log.tracef(m => m("  setPosition (%i, %i)", line + 1, col));

        let cursor =
          Vim.Cursor.create(
            ~line=Index.fromOneBased(line + 1),
            ~column=Index.fromZeroBased(col),
          );

        /*GlobalContext.current().dispatch(
            Actions.EditorScrollToLine(editorId, topVisibleLine),
          );
          GlobalContext.current().dispatch(
            Actions.EditorScrollToColumn(editorId, leftVisibleColumn),
          );*/
        onCursorChange(cursor);
      };
    };
  };

  let horizontalScrollBarStyle =
    Style.[
      position(`Absolute),
      bottom(0),
      left(int_of_float(layout.lineNumberWidthInPixels)),
      height(Constants.default.scrollBarThickness),
      width(int_of_float(layout.bufferWidthInPixels)),
    ];

  <View
    style={Styles.bufferViewClipped(bufferPixelWidth)} onMouseUp onMouseWheel>
    <OpenGL
      style={Styles.bufferViewClipped(bufferPixelWidth)}
      render={(transform, _ctx) => {
        let count = lineCount;
        let height = metrics.pixelHeight;
        let rowHeight = metrics.lineHeight;

        drawCurrentLineHighlight(
          cursorPosition.line,
          ~transform,
          ~gutterWidth,
          ~metrics,
          ~scrollY,
          ~lineHeight={editorFont.measuredHeight},
          ~theme,
        );

        renderRulers(
          ~rulers,
          ~editorFont,
          ~gutterWidth,
          ~scrollX,
          ~scrollY,
          ~transform,
          ~metrics,
          ~theme,
        );

        GutterView.render(
          ~showLineNumbers,
          ~transform,
          ~lineNumberWidth,
          ~height,
          ~theme,
          ~scrollY,
          ~rowHeight,
          ~count,
          ~editorFont,
          ~cursorLine=Index.toZeroBased(cursorPosition.line),
          ~diffMarkers,
        );

        let bufferPositionToPixel =
          bufferPositionToPixel(
            ~gutterWidth,
            ~scrollX,
            ~scrollY,
            ~editorFont,
          );

        ContentView.render(
          ~scrollY,
          ~rowHeight,
          ~height,
          ~count,
          ~transform,
          ~buffer,
          ~editor,
          ~gutterWidth,
          ~leftVisibleColumn,
          ~theme,
          ~diagnosticsMap,
          ~selectionRanges,
          ~matchingPairs,
          ~bufferHighlights,
          ~cursorPosition,
          ~definition,
          ~layout,
          ~bufferSyntaxHighlights,
          ~editorFont,
          ~shouldRenderWhitespace,
        );

        if (shouldRenderIndentGuides) {
          IndentLineRenderer.render(
            ~transform,
            ~buffer,
            ~startLine=topVisibleLine - 1,
            ~endLine=bottomVisibleLine + 1,
            ~lineHeight=editorFont.measuredHeight,
            ~fontWidth=editorFont.measuredWidth,
            ~cursorLine=Index.toZeroBased(cursorPosition.line),
            ~theme,
            ~indentationSettings=indentation,
            ~bufferPositionToPixel,
            ~showActive=shouldHighlightActiveIndentGuides,
            (),
          );
        };
      }}
    />
    <CursorView
      buffer
      mode
      isActiveSplit
      cursorPosition
      editor
      editorFont
      gutterWidth
    />
    <View style=horizontalScrollBarStyle>
      <EditorHorizontalScrollbar
        editor
        metrics
        width={int_of_float(layout.bufferWidthInPixels)}
        theme
      />
    </View>
  </View>;
};
