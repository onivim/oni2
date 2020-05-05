open EditorCoreTypes;
open Revery.UI;
open Revery.Math;

open Oni_Core;

open Helpers;

module Log = (val Log.withNamespace("Oni2.Editor.SurfaceView"));

module Config = EditorConfiguration;

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

let drawCurrentLineHighlight = (~context, ~colors: Colors.t, line) =>
  Draw.lineHighlight(~context, ~color=colors.lineHighlightBackground, line);

let renderRulers = (~context, ~colors: Colors.t, rulers) =>
  rulers
  |> List.map(bufferPositionToPixel(~context, 0))
  |> List.map(fst)
  |> List.iter(Draw.ruler(~context, ~color=colors.rulerForeground));

let%component make =
              (
                ~onScroll,
                ~buffer,
                ~editor,
                ~colors,
                ~topVisibleLine,
                ~onCursorChange,
                ~cursorPosition: Location.t,
                ~editorFont: Service_Font.font,
                ~leftVisibleColumn,
                ~diagnosticsMap,
                ~selectionRanges,
                ~matchingPairs,
                ~bufferHighlights,
                ~definition,
                ~bufferSyntaxHighlights,
                ~bottomVisibleLine,
                ~mode,
                ~isActiveSplit,
                ~gutterWidth,
                ~bufferWidthInCharacters,
                ~windowIsFocused,
                ~config,
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

  let {scrollX, scrollY, _}: Editor.t = editor;

  let onMouseUp = (evt: NodeEvents.mouseButtonEventParams) => {
    Log.trace("editorMouseUp");

    switch (elementRef^) {
    | None => ()
    | Some(r) =>
      let (minX, minY, _, _) = r#getBoundingBox() |> BoundingBox2d.getBounds;

      let relY = evt.mouseY -. minY;
      let relX = evt.mouseX -. minX;

      let (line, col) =
        Editor.pixelPositionToBufferLineByte(
          ~buffer,
          ~pixelX=relX,
          ~pixelY=relY,
          editor,
        );

      Log.tracef(m => m("  topVisibleLine is %i", topVisibleLine));
      Log.tracef(m => m("  setPosition (%i, %i)", line + 1, col));

      let cursor =
        Vim.Cursor.create(
          ~line=Index.fromOneBased(line + 1),
          ~column=Index.fromZeroBased(col),
        );

      onCursorChange(cursor);
    };
  };

  <View
    ref={node => elementRef := Some(node)}
    style={Styles.bufferViewClipped(
      gutterWidth,
      float(Editor.(editor.pixelWidth)) -. gutterWidth,
    )}
    onMouseUp
    onMouseWheel>
    <Canvas
      style={Styles.bufferViewClipped(
        0.,
        float(Editor.(editor.pixelWidth)) -. gutterWidth,
      )}
      render={canvasContext => {
        let context =
          Draw.createContext(
            ~canvasContext,
            ~width=editor.pixelWidth,
            ~height=editor.pixelHeight,
            ~scrollX,
            ~scrollY,
            ~lineHeight=editorFont.measuredHeight,
            ~editorFont,
          );

        drawCurrentLineHighlight(~context, ~colors, cursorPosition.line);

        renderRulers(~context, ~colors, Config.rulers.get(config));

        ContentView.render(
          ~context,
          ~count=lineCount,
          ~buffer,
          ~leftVisibleColumn,
          ~colors,
          ~diagnosticsMap,
          ~selectionRanges,
          ~matchingPairs,
          ~bufferHighlights,
          ~cursorPosition,
          ~definition,
          ~bufferSyntaxHighlights,
          ~shouldRenderWhitespace=Config.renderWhitespace.get(config),
          ~bufferWidthInCharacters,
        );

        if (Config.renderIndentGuides.get(config)) {
          IndentLineRenderer.render(
            ~context,
            ~buffer,
            ~startLine=topVisibleLine - 1,
            ~endLine=bottomVisibleLine + 1,
            ~cursorPosition,
            ~colors,
            ~showActive=Config.highlightActiveIndentGuide.get(config),
            indentation,
          );
        };
      }}
    />
    <CursorView
      config
      editor
      scrollX
      scrollY
      editorFont
      buffer
      mode
      cursorPosition
      isActiveSplit
      windowIsFocused
      colors
    />
    <View style=Styles.horizontalScrollBar>
      <EditorHorizontalScrollbar editor width={editor.pixelWidth} colors />
    </View>
  </View>;
};
