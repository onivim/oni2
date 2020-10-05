open EditorCoreTypes;
open Revery.UI;
open Revery.Math;

open Oni_Core;

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

module Constants = {
  let hoverTime = 1.0;
};

let drawCurrentLineHighlight =
    (~context, ~colors: Colors.t, line: EditorCoreTypes.LineNumber.t) =>
  Draw.lineHighlight(~context, ~color=colors.lineHighlightBackground, line);

let renderRulers = (~context: Draw.context, ~colors: Colors.t, rulers) => {
  let characterWidth = Editor.characterWidthInPixels(context.editor);
  let scrollX = Editor.scrollX(context.editor);
  rulers
  |> List.map(pos => characterWidth *. float(pos) -. scrollX)
  |> List.iter(Draw.ruler(~context, ~color=colors.rulerForeground));
};

let%component make =
              (
                ~buffer,
                ~editor,
                ~colors,
                ~dispatch,
                ~topVisibleLine,
                ~onCursorChange,
                ~cursorPosition: CharacterPosition.t,
                ~editorFont: Service_Font.font,
                ~leftVisibleColumn,
                ~diagnosticsMap,
                ~selectionRanges,
                ~matchingPairs,
                ~bufferHighlights,
                ~languageSupport,
                ~bufferSyntaxHighlights,
                ~bottomVisibleLine,
                ~maybeYankHighlights,
                ~mode,
                ~isActiveSplit,
                ~gutterWidth,
                ~bufferPixelWidth,
                ~bufferWidthInCharacters,
                ~windowIsFocused,
                ~config,
                (),
              ) => {
  let%hook maybeBbox = React.Hooks.ref(None);
  let%hook hoverTimerActive = React.Hooks.ref(false);
  let%hook lastMousePosition = React.Hooks.ref(None);
  let%hook (hoverTimer, resetHoverTimer) =
    Hooks.timer(~active=hoverTimerActive^, ());

  let lineCount = editor |> Editor.totalViewLines;
  let indentation = Buffer.getIndentation(buffer);

  let onMouseWheel = (wheelEvent: NodeEvents.mouseWheelEventParams) =>
    dispatch(
      Msg.EditorMouseWheel({
        deltaY: wheelEvent.deltaY *. (-1.),
        deltaX: wheelEvent.deltaX,
        shiftKey: wheelEvent.shiftKey,
      }),
    );

  let getMaybeLocationFromMousePosition = (mouseX, mouseY) => {
    maybeBbox^
    |> Option.map(bbox => {
         let (minX, minY, _, _) = bbox |> BoundingBox2d.getBounds;

         let relX = mouseX -. minX;
         let relY = mouseY -. minY;

         Editor.Slow.pixelPositionToBytePosition(
           // #2463: When we're in insert mode, clicking past the end of the line
           // should move the cursor past the last byte.
           ~allowPast=mode == Vim.Mode.Insert,
           ~buffer,
           ~pixelX=relX,
           ~pixelY=relY,
           editor,
         );
       });
  };

  let onMouseMove = (evt: NodeEvents.mouseMoveEventParams) => {
    getMaybeLocationFromMousePosition(evt.mouseX, evt.mouseY)
    |> Option.iter(bytePosition => {
         dispatch(Msg.MouseMoved({bytePosition: bytePosition}))
       });
    hoverTimerActive := true;
    lastMousePosition := Some((evt.mouseX, evt.mouseY));
    resetHoverTimer();
  };

  let onMouseLeave = _ => {
    hoverTimerActive := false;
    lastMousePosition := None;
    resetHoverTimer();
  };

  // Usually the If hook compares a value to a prior version of itself
  // However, here we just want to compare it to a constant value,
  // so we discard the second argument to the function.
  let%hook () =
    Hooks.effect(
      If(
        (t, _) => Revery.Time.toFloatSeconds(t) > Constants.hoverTime,
        hoverTimer,
      ),
      () => {
        lastMousePosition^
        |> Utility.OptionEx.flatMap(((mouseX, mouseY)) =>
             getMaybeLocationFromMousePosition(mouseX, mouseY)
           )
        |> Option.iter(bytePosition => {
             dispatch(Msg.MouseHovered({bytePosition: bytePosition}))
           });
        hoverTimerActive := false;
        resetHoverTimer();
        None;
      },
    );

  let onMouseUp = (evt: NodeEvents.mouseButtonEventParams) => {
    Log.trace("editorMouseUp");

    getMaybeLocationFromMousePosition(evt.mouseX, evt.mouseY)
    |> Option.iter(bytePosition => {
         Log.tracef(m => m("  topVisibleLine is %i", topVisibleLine));
         Log.tracef(m =>
           m("  setPosition (%s)", BytePosition.show(bytePosition))
         );
         onCursorChange(bytePosition);
       });
  };

  let pixelWidth = Editor.visiblePixelWidth(editor);
  let pixelHeight = Editor.visiblePixelHeight(editor);

  let yankHighlightElement =
    maybeYankHighlights
    |> Option.map((highlights: Editor.yankHighlight) =>
         <YankHighlights
           config
           key={highlights.key}
           pixelRanges={highlights.pixelRanges}
         />
       )
    |> Option.value(~default=React.empty);

  <View
    onBoundingBoxChanged={bbox => maybeBbox := Some(bbox)}
    style={Styles.bufferViewClipped(
      gutterWidth,
      float(pixelWidth) -. gutterWidth,
    )}
    onMouseUp
    onMouseMove
    onMouseLeave
    onMouseWheel>
    <Canvas
      style={Styles.bufferViewClipped(0., float(pixelWidth) -. gutterWidth)}
      render={(canvasContext, _) => {
        let context =
          Draw.createContext(
            ~canvasContext,
            ~width=pixelWidth,
            ~height=pixelHeight,
            ~editor,
            ~editorFont,
          );

        drawCurrentLineHighlight(~context, ~colors, cursorPosition.line);

        renderRulers(~context, ~colors, Config.rulers.get(config));

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

        ContentView.render(
          ~context,
          ~count=lineCount,
          ~buffer,
          ~editor,
          ~leftVisibleColumn,
          ~colors,
          ~diagnosticsMap,
          ~selectionRanges,
          ~matchingPairs,
          ~bufferHighlights,
          ~cursorPosition,
          ~languageSupport,
          ~bufferSyntaxHighlights,
          ~shouldRenderWhitespace=Config.renderWhitespace.get(config),
          ~bufferWidthInCharacters,
          ~bufferWidthInPixels=bufferPixelWidth,
        );

        if (Config.scrollShadow.get(config)) {
          let () =
            ScrollShadow.renderVertical(
              ~editor,
              ~width=float(bufferPixelWidth),
              ~context,
            );
          let () =
            ScrollShadow.renderHorizontal(
              ~editor,
              ~width=float(bufferPixelWidth),
              ~context,
            );
          ();
        };
      }}
    />
    yankHighlightElement
    <CursorView
      config
      editor
      editorFont
      mode
      cursorPosition
      isActiveSplit
      windowIsFocused
      colors
    />
  </View>;
};
