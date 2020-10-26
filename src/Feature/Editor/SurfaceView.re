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

let drawCurrentLineHighlight = (~context, ~colors: Colors.t, viewLine: int) =>
  Draw.lineHighlight(
    ~context,
    ~color=colors.lineHighlightBackground,
    viewLine,
  );

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
                ~changeMode,
                ~cursorPosition: CharacterPosition.t,
                ~editorFont: Service_Font.font,
                ~diagnosticsMap,
                ~selectionRanges,
                ~matchingPairs,
                ~bufferHighlights,
                ~languageSupport,
                ~languageConfiguration,
                ~bufferSyntaxHighlights,
                ~maybeYankHighlights,
                ~mode,
                ~isActiveSplit,
                ~gutterWidth,
                ~bufferPixelWidth,
                ~windowIsFocused,
                ~config,
                (),
              ) => {
  let%hook maybeBbox = React.Hooks.ref(None);
  let%hook hoverTimerActive = React.Hooks.ref(false);
  let%hook lastMousePosition = React.Hooks.ref(None);
  let%hook (hoverTimer, resetHoverTimer) =
    Hooks.timer(~name="Mouse Hover Timer", ~active=hoverTimerActive^, ());

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
           ~allowPast=Vim.Mode.isInsert(mode),
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
         Log.tracef(m =>
           m("  setPosition (%s)", BytePosition.show(bytePosition))
         );
         let newMode =
           if (Vim.Mode.isInsert(Editor.mode(editor))) {
             Vim.Mode.Insert({cursors: [bytePosition]});
           } else {
             Vim.Mode.Normal({cursor: bytePosition});
           };
         changeMode(newMode);
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

        let maybeCursorBytePosition =
          Editor.characterToByte(cursorPosition, editor);
        maybeCursorBytePosition
        |> Option.iter(bytePosition => {
             let viewLine =
               Editor.bufferBytePositionToViewLine(bytePosition, editor);
             drawCurrentLineHighlight(~context, ~colors, viewLine);
           });

        renderRulers(~context, ~colors, Config.rulers.get(config));

        if (Config.renderIndentGuides.get(config)) {
          IndentLineRenderer.render(
            ~context,
            ~buffer,
            ~startLine=Editor.getTopVisibleBufferLine(editor),
            ~endLine=Editor.getBottomVisibleBufferLine(editor),
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
          ~colors,
          ~diagnosticsMap,
          ~selectionRanges,
          ~matchingPairs,
          ~bufferHighlights,
          ~cursorPosition,
          ~languageSupport,
          ~languageConfiguration,
          ~bufferSyntaxHighlights,
          ~shouldRenderWhitespace=Config.renderWhitespace.get(config),
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
