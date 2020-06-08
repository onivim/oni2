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
                ~buffer,
                ~editor,
                ~colors,
                ~dispatch,
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
                ~bufferPixelWidth,
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
    dispatch(Msg.EditorMouseWheel({deltaWheel: wheelEvent.deltaY *. (-1.)}));

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

        let shadowSize = 18.;
        if (editor.scrollX > 1.) {
          Draw.shadow(
            ~context,
            ~x=0.,
            ~y=0.,
            ~width=shadowSize,
            ~height=float(editor.pixelHeight),
          );
        };

        if (editor.scrollY > 1.) {
          Draw.shadow2(
            ~context,
            ~x=0.,
            ~y=-5.,
            ~width=float(editor.pixelWidth),
            ~height=shadowSize,
          );
        };

        if (editor.scrollX
            +. float(bufferPixelWidth)
            < float(Editor.getTotalWidthInPixels(editor))) {
          let () =
            Draw.shadow3(
              ~context,
              ~x=float(bufferPixelWidth) -. shadowSize -. 1.0,
              ~y=0.,
              ~width=shadowSize,
              ~height=float(editor.pixelHeight),
            );
          ();
        };

        if (editor.scrollY
            +. float(editor.pixelHeight)
            < float(Editor.getTotalHeightInPixels(editor))) {
          let () =
            Draw.shadow4(
              ~context,
              ~x=0.,
              ~y=float(editor.pixelHeight) -. shadowSize -. 1.0,
              ~width=float(editor.pixelWidth),
              ~height=shadowSize,
            );
          ();
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
  </View>;
};
