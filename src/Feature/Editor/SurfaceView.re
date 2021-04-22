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
                ~vim,
                ~languageSupport,
                ~languageConfiguration,
                ~bufferSyntaxHighlights,
                ~maybeYankHighlights,
                ~mode,
                ~snippets: Feature_Snippets.model,
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

  //let inlineElements = Editor.getInlineElements(editor);

  let topVisibleLine = Editor.getTopVisibleBufferLine(editor);
  let bottomVisibleLine = Editor.getBottomVisibleBufferLine(editor);

  let rec getInlineElements = (acc: list(Revery.UI.element), lines) =>
    switch (lines) {
    | [] => acc
    | [line, ...tail] =>
      let isVisible = line >= topVisibleLine && line <= bottomVisibleLine;
      getInlineElements(
        [
          <InlineElementView.Container
            config
            uiFont
            theme
            editor
            line
            dispatch
            isVisible
          />,
          ...acc,
        ],
        tail,
      );
    };

  let linesWithElements = Editor.linesWithInlineElements(editor);

  let lensElements = getInlineElements([], linesWithElements);

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
         dispatch(
           Msg.EditorMouseDown({altKey: evt.ctrlKey, time, pixelX, pixelY}),
         )
       });
  };

  let onMouseUp = (evt: NodeEvents.mouseButtonEventParams) => {
    getMaybeLocationFromMousePosition(evt.mouseX, evt.mouseY)
    |> Option.iter(((pixelX, pixelY, time)) => {
         dispatch(
           Msg.EditorMouseUp({altKey: evt.ctrlKey, time, pixelX, pixelY}),
         )
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

  let cursors =
    Editor.cursors(editor)
    |> List.filter_map(pos => Editor.byteToCharacter(pos, editor))
    |> List.map(cursorPosition => {
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
       })
    |> React.listToElement;

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

        snippets
        |> Feature_Snippets.session
        |> Option.iter(SnippetVisualizer.draw(~colors, ~config, ~context));

        ContentView.render(
          ~context,
          ~buffer,
          ~editor,
          ~colors,
          ~diagnosticsMap,
          ~selectionRanges,
          ~matchingPairs,
          ~vim,
          ~cursorPosition,
          ~languageSupport,
          ~languageConfiguration,
          ~bufferSyntaxHighlights,
          ~shouldRenderWhitespace=Config.renderWhitespace.get(config),
        );

        if (Config.scrollShadow.get(config)) {
          let () =
            ScrollShadow.renderVertical(
              ~color=colors.shadow,
              ~editor,
              ~width=float(bufferPixelWidth),
              ~context,
            );
          let () =
            ScrollShadow.renderHorizontal(
              ~color=colors.shadow,
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
    cursors
  </View>;
};
