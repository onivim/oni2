open EditorCoreTypes;
open Revery.UI;
open Revery.Math;

open Oni_Core;

open Helpers;

module Log = (val Log.withNamespace("Oni2.Editor.SurfaceView"));

module Styles = {
  open Style;

  let bufferViewClipped = (offsetLeft, bufferPixelWidth) => [
    overflow(`Hidden),
    position(`Absolute),
    top(0),
    left(int_of_float(offsetLeft)),
    width(int_of_float(bufferPixelWidth)),
    bottom(0),
  ];

  let horizontalScrollBar = [
    position(`Absolute),
    bottom(0),
    left(0),
    right(0),
    height(Constants.scrollBarThickness),
  ];
};

let drawCurrentLineHighlight = (~context, ~theme: Theme.t, line) =>
  Draw.lineHighlight(
    ~context,
    ~color=theme.editorLineHighlightBackground,
    line,
  );

let renderRulers = (~context, ~theme: Theme.t, rulers) =>
  rulers
  |> List.map(bufferPositionToPixel(~context, 0))
  |> List.map(fst)
  |> List.iter(Draw.ruler(~context, ~color=theme.editorRulerForeground));

let%component make =
              (
                ~onScroll,
                ~buffer,
                ~editor,
                ~metrics,
                ~theme,
                ~topVisibleLine,
                ~onCursorChange,
                ~cursorPosition: Location.t,
                ~rulers,
                ~editorFont: EditorFont.t,
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
                ~gutterWidth,
                ~bufferWidthInCharacters,
                (),
              ) => {
  let%hook elementRef = React.Hooks.ref(None);

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

    switch (elementRef^) {
    | None => ()
    | Some(r) =>
      let (minX, minY, _, _) = r#getBoundingBox() |> BoundingBox2d.getBounds;

      let relY = evt.mouseY -. minY;
      let relX = evt.mouseX -. minX;

      let numberOfLines = Buffer.getNumberOfLines(buffer);
      let (line, col) =
        Editor.pixelPositionToLineColumn(editor, metrics, relX, relY);

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

  <View
    ref={node => elementRef := Some(node)}
    style={Styles.bufferViewClipped(
      gutterWidth,
      float(metrics.pixelWidth) -. gutterWidth,
    )}
    onMouseUp
    onMouseWheel>
    <Canvas
      style={Styles.bufferViewClipped(
        0.,
        float(metrics.pixelWidth) -. gutterWidth,
      )}
      render={canvasContext => {
        let context =
          Draw.createContext(
            ~canvasContext,
            ~width=metrics.pixelWidth,
            ~height=metrics.pixelHeight,
            ~scrollX=editor.scrollX,
            ~scrollY=editor.scrollY,
            ~lineHeight=editorFont.measuredHeight,
            ~editorFont,
          );

        drawCurrentLineHighlight(~context, ~theme, cursorPosition.line);

        renderRulers(~context, ~theme, rulers);

        ContentView.render(
          ~context,
          ~count=lineCount,
          ~buffer,
          ~leftVisibleColumn,
          ~theme,
          ~diagnosticsMap,
          ~selectionRanges,
          ~matchingPairs,
          ~bufferHighlights,
          ~cursorPosition,
          ~definition,
          ~bufferSyntaxHighlights,
          ~shouldRenderWhitespace,
          ~bufferWidthInCharacters,
        );

        if (shouldRenderIndentGuides) {
          IndentLineRenderer.render(
            ~context,
            ~buffer,
            ~startLine=topVisibleLine - 1,
            ~endLine=bottomVisibleLine + 1,
            ~cursorPosition,
            ~theme,
            ~showActive=shouldHighlightActiveIndentGuides,
            indentation,
          );
        };
      }}
    />
    <CursorView buffer mode isActiveSplit cursorPosition editor editorFont />
    <View style=Styles.horizontalScrollBar>
      <EditorHorizontalScrollbar
        editor
        metrics
        width={metrics.pixelWidth}
        theme
      />
    </View>
  </View>;
};
