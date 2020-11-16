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
                ~uiFont,
                ~theme,
                (),
              ) => {
  let%hook maybeBbox = React.Hooks.ref(None);

  let indentation = Buffer.getIndentation(buffer);

  let inlineElements = Editor.getInlineElements(editor);

  let lensElements =
    inlineElements
    |> List.map((inlineElement: InlineElements.element) => {
         let line = inlineElement.line;
         let uniqueId = inlineElement.uniqueId;
         let elem = inlineElement.view(~theme, ~uiFont);
         let inlineKey = inlineElement.key;
         let hidden = inlineElement.hidden;

         <InlineElementView
           config
           key={inlineElement.reconcilerKey}
           inlineKey
           uniqueId
           dispatch
           lineNumber=line
           hidden
           editor>
           <elem />
         </InlineElementView>;
       });

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
         let time = Revery.Time.now();
         (relX, relY, time);
       });
  };

  let onMouseMove = (evt: NodeEvents.mouseMoveEventParams) => {
    getMaybeLocationFromMousePosition(evt.mouseX, evt.mouseY)
    |> Option.iter(((pixelX, pixelY, time)) => {
         dispatch(Msg.EditorMouseMoved({time, pixelX, pixelY}))
       });
  };

  let onMouseEnter = _evt => {
    dispatch(Msg.EditorMouseEnter);
  };

  let onMouseLeave = _evt => {
    dispatch(Msg.EditorMouseLeave);
  };

  let onMouseDown = (evt: NodeEvents.mouseButtonEventParams) => {
    getMaybeLocationFromMousePosition(evt.mouseX, evt.mouseY)
    |> Option.iter(((pixelX, pixelY, time)) => {
         dispatch(Msg.EditorMouseDown({time, pixelX, pixelY}))
       });
  };

  let onMouseUp = (evt: NodeEvents.mouseButtonEventParams) => {
    getMaybeLocationFromMousePosition(evt.mouseX, evt.mouseY)
    |> Option.iter(((pixelX, pixelY, time)) => {
         dispatch(Msg.EditorMouseUp({time, pixelX, pixelY}))
       });
  };

  let pixelWidth = Editor.visiblePixelWidth(editor);
  let pixelHeight = Editor.visiblePixelHeight(editor);

  let yankHighlightElement =
    maybeYankHighlights
    |> Option.map((highlights: Editor.yankHighlight) =>
         <YankHighlights
           config
           opacity={Component_Animation.get(highlights.opacity)}
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
    onMouseDown
    onMouseUp
    onMouseMove
    onMouseEnter
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
    {lensElements |> React.listToElement}
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
