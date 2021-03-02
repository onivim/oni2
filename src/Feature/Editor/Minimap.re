/*
 * Minimap.re
 *
 * Component that handles Minimap rendering
 */

open EditorCoreTypes;
open Oni_Core;
module OptionEx = Utility.OptionEx;
open Revery.Draw;
open Revery.UI;

module Diagnostic = Feature_Diagnostics.Diagnostic;

module Constants = {
  include Constants;

  let leftMargin = 2.;
  let diffMarkerWidth = 2.;
  let gutterMargin = 2.;
  let gutterWidth = diffMarkerWidth +. gutterMargin;
};

module Styles = {
  open Style;
  let absolute = [
    position(`Absolute),
    top(0),
    bottom(0),
    left(0),
    right(0),
  ];

  let absoluteWithCursor = [
    cursor(Revery.MouseCursors.pointer),
    ...absolute,
  ];
  let color = Revery.Color.rgba(0., 0., 0., 0.5);
  let container = backgroundColor => [
    Style.backgroundColor(backgroundColor),
    ...absoluteWithCursor,
  ];
};

let lineStyle = Style.[position(`Absolute), top(0)];

let minimapPaint = Skia.Paint.make();

let renderLine =
    (
      ~scaleFactor,
      shouldHighlight,
      canvasContext,
      yOffset,
      tokens: list(BufferViewTokenizer.t),
    ) => {
  let f = (token: BufferViewTokenizer.t) => {
    switch (token.tokenType) {
    | Text =>
      let startPosition = CharacterIndex.toInt(token.startIndex);

      let x = token.startPixel *. scaleFactor;
      let endX = token.endPixel *. scaleFactor;
      let height = float(Constants.minimapCharacterHeight);
      let width = endX -. x;

      let emphasis = shouldHighlight(startPosition);
      let color =
        emphasis ? token.color : Revery.Color.multiplyAlpha(0.5, token.color);

      let offset = 1.0;
      let halfOffset = offset /. 2.0;

      let x =
        (emphasis ? x -. halfOffset : x)
        +. Constants.leftMargin
        +. Constants.gutterWidth;
      let y = yOffset;
      let width = emphasis ? width +. offset : width;

      Skia.Paint.setColor(minimapPaint, Revery.Color.toSkia(color));
      CanvasContext.drawRectLtwh(
        ~top=y,
        ~left=x,
        ~paint=minimapPaint,
        ~width,
        ~height,
        canvasContext,
      );
    | _ => ()
    };
  };

  List.iter(f, tokens);
};

let getMinimapSize = (editor: Editor.t) => {
  let currentViewSize = Editor.getVisibleView(editor);
  let totalLines = Editor.totalViewLines(editor);

  totalLines < currentViewSize ? 0 : currentViewSize + 1;
};

type captureState = {
  bbox: Revery.Math.BoundingBox2d.t,
  offset: float,
};

let%component make =
              (
                ~dispatch: Msg.t => unit,
                ~editor: Editor.t,
                ~config: Config.resolver,
                ~cursorPosition: CharacterPosition.t,
                ~width: int,
                ~height: int,
                ~count,
                ~diagnostics,
                ~maybeYankHighlights: option(Editor.yankHighlight),
                ~getTokensForLine: int => list(BufferViewTokenizer.t),
                ~selection:
                   Hashtbl.t(
                     EditorCoreTypes.LineNumber.t,
                     list(ByteRange.t),
                   ),
                ~showSlider,
                ~colors: Colors.t,
                ~vim,
                ~languageSupport,
                ~diffMarkers,
                (),
              ) => {
  let rowHeight =
    float(Constants.minimapCharacterHeight + Constants.minimapLineSpacing);

  let scrollY = Editor.minimapScrollY(editor);

  let editorScrollY = Editor.scrollY(editor);
  let topViewLine = editorScrollY /. Editor.lineHeightInPixels(editor);
  let thumbTop = rowHeight *. topViewLine -. scrollY;
  let thumbSize = rowHeight *. float(getMinimapSize(editor));

  let%hook (maybeBbox, setBbox) = Hooks.state(None);
  let%hook (maybeHoverLine, setHoverLine) = Hooks.state(None);

  let isHovering = maybeHoverLine != None;

  let getRelativeMousePosition = (mouseY: float) => {
    maybeBbox
    |> Option.map(bbox => {
         let (_x, y, _width, _height) =
           Revery.Math.BoundingBox2d.getBounds(bbox);
         mouseY -. y;
       });
  };

  let getScrollTo = (~offset=0., screenMouseY: float) => {
    getRelativeMousePosition(screenMouseY)
    |> Option.map(mouseY => {
         let (_pixelX, pixelY) =
           Editor.unprojectToPixel(
             ~pixelX=0.,
             ~pixelY=mouseY -. offset,
             ~pixelWidth=width,
             ~pixelHeight=height,
             editor,
           );
         pixelY;
       });
  };

  let%hook (captureMouse, _captureState) =
    Hooks.mouseCapture(
      ~onMouseMove=
        (state, evt) => {
          evt.mouseY
          |> getScrollTo(~offset=state.offset)
          |> OptionEx.tap(scrollTo => {
               dispatch(Msg.MinimapDragged({newPixelScrollY: scrollTo}))
             })
          |> Option.map(_ => state)
        },
      ~onMouseUp=(_, _) => None,
      (),
    );

  let onMouseDown = (evt: NodeEvents.mouseButtonEventParams) => {
    evt.mouseY
    |> getRelativeMousePosition
    |> Option.iter((position: float) =>
         if (evt.button == Revery.MouseButton.BUTTON_LEFT) {
           let line = int_of_float((position +. scrollY) /. rowHeight);
           if (position < thumbTop) {
             dispatch(Msg.MinimapClicked({viewLine: line}));
           } else if (position > thumbTop +. thumbSize) {
             dispatch(Msg.MinimapClicked({viewLine: line}));
           } else {
             maybeBbox
             |> Option.iter(bbox => {
                  captureMouse({bbox, offset: position -. thumbTop})
                });
           };
         }
       );
  };

  let setHighlightLine = (evt: NodeEvents.mouseMoveEventParams) => {
    evt.mouseY
    |> getRelativeMousePosition
    |> Option.iter((position: float) => {
         let line = int_of_float((position +. scrollY) /. rowHeight);
         setHoverLine(_ => Some(line));
       });
  };

  let onMouseMove = (evt: NodeEvents.mouseMoveEventParams) => {
    setHighlightLine(evt);
  };

  let onMouseOver = evt => {
    setHighlightLine(evt);
  };

  let onMouseLeave = _ => {
    setHoverLine(_ => None);
  };

  let backgroundColor =
    isHovering
      ? colors.minimapBackground
      : colors.minimapBackground |> Revery.Color.multiplyAlpha(0.8);

  let sliderBackground =
    isHovering
      ? colors.minimapSliderHoverBackground : colors.minimapSliderBackground;

  // Convert an editor surface pixel range (post-scroll) to a
  // minimap pixel range. For now, this just has per-line fidelity.
  let mapPixelRange = ({start, stop}: PixelRange.t) => {
    let editorPixelYToMinimapPixelY = pixelY => {
      let scaleFactor = rowHeight /. Editor.lineHeightInPixels(editor);
      pixelY *. scaleFactor +. thumbTop;
    };

    PixelRange.{
      start: PixelPosition.{x: 0., y: editorPixelYToMinimapPixelY(start.y)},
      stop:
        PixelPosition.{
          x: float(width),
          y: editorPixelYToMinimapPixelY(stop.y),
        },
    };
  };

  let yankHighlightElement =
    maybeYankHighlights
    |> Option.map(({key, pixelRanges, opacity}: Editor.yankHighlight) => {
         let pixelRanges = pixelRanges |> List.map(mapPixelRange);
         let opacity = Component_Animation.get(opacity);

         <YankHighlights opacity config key pixelRanges />;
       })
    |> Option.value(~default=React.empty);

  <View
    style={Styles.container(backgroundColor)}
    onMouseDown
    onMouseMove
    onMouseOver
    onMouseLeave
    onBoundingBoxChanged={bbox => setBbox(_ => Some(bbox))}>
    <Canvas
      style=Styles.absolute
      render={(canvasContext, _) => {
        Skia.Paint.setColor(
          minimapPaint,
          Revery.Color.toSkia(sliderBackground),
        );
        if (showSlider) {
          /* Draw slider/viewport */
          CanvasContext.drawRectLtwh(
            ~left=0.,
            ~top=thumbTop,
            ~height=thumbSize,
            ~width=float(width),
            ~paint=minimapPaint,
            canvasContext,
          );
        };

        /* Draw hover line */

        maybeHoverLine
        |> Option.iter(hoverLine => {
             CanvasContext.drawRectLtwh(
               ~left=Constants.leftMargin,
               ~top=rowHeight *. float(hoverLine) -. scrollY,
               ~height=float(Constants.minimapCharacterHeight),
               ~width=float(width),
               ~paint=minimapPaint,
               canvasContext,
             )
           });

        Skia.Paint.setColor(
          minimapPaint,
          Revery.Color.toSkia(colors.lineHighlightBackground),
        );
        /* Draw cursor line */
        Editor.characterToByte(cursorPosition, editor)
        |> Option.iter(bytePosition => {
             let viewLine =
               Editor.bufferBytePositionToViewLine(bytePosition, editor);
             CanvasContext.drawRectLtwh(
               ~left=Constants.leftMargin,
               ~top=rowHeight *. float(viewLine) -. scrollY,
               ~height=float(Constants.minimapCharacterHeight),
               ~width=float(width),
               ~paint=minimapPaint,
               canvasContext,
             );
           });

        let renderRange = (~color, ~offset, range: ByteRange.t) =>
          {let maybeCharacterStart =
             Editor.byteToCharacter(range.start, editor);
           let maybeCharacterStop =
             Editor.byteToCharacter(range.stop, editor);

           OptionEx.iter2(
             (characterStart, characterStop) => {
               let startX =
                 CharacterPosition.(
                   float(CharacterIndex.toInt(characterStart.character))
                   *. float(Constants.minimapCharacterWidth)
                   +. Constants.leftMargin
                   +. Constants.gutterWidth
                 );
               let endX =
                 CharacterPosition.(
                   float(CharacterIndex.toInt(characterStop.character))
                   *. float(Constants.minimapCharacterWidth)
                 );

               Skia.Paint.setColor(minimapPaint, Revery.Color.toSkia(color));
               CanvasContext.drawRectLtwh(
                 ~left=startX -. 1.0,
                 ~top=offset -. 1.0,
                 ~height=float(Constants.minimapCharacterHeight) +. 2.0,
                 ~width=endX -. startX +. 2.,
                 ~paint=minimapPaint,
                 canvasContext,
               );
             },
             maybeCharacterStart,
             maybeCharacterStop,
           )};

        let renderUnderline = (~color, ~offset, range: CharacterRange.t) =>
          {let startX =
             float(CharacterIndex.toInt(range.start.character))
             *. float(Constants.minimapCharacterWidth)
             +. Constants.leftMargin
             +. Constants.gutterWidth;
           let endX =
             float(CharacterIndex.toInt(range.stop.character))
             *. float(Constants.minimapCharacterWidth);

           Skia.Paint.setColor(minimapPaint, Revery.Color.toSkia(color));
           CanvasContext.drawRectLtwh(
             ~left=startX -. 1.0,
             ~top=offset +. float(Constants.minimapCharacterHeight),
             ~height=1.0,
             ~width=endX -. startX +. 2.,
             ~paint=minimapPaint,
             canvasContext,
           )};

        let scaleFactor = Editor.getMinimapWidthScaleFactor(editor);
        ImmediateList.render(
          ~scrollY,
          ~rowHeight,
          ~height=float(height),
          ~count,
          ~render=
            (item, offset) => {
              /* draw selection */
              let index = EditorCoreTypes.LineNumber.ofZeroBased(item);
              switch (Hashtbl.find_opt(selection, index)) {
              | None => ()
              | Some(v) =>
                let color = colors.minimapSelectionHighlight;
                List.iter(renderRange(~color, ~offset), v);
              };

              let tokens = getTokensForLine(item);
              let bufferId = Editor.getBufferId(editor);

              let searchHighlightRanges =
                Feature_Vim.getSearchHighlightsByLine(
                  ~bufferId,
                  ~line=index,
                  vim,
                )
                |> List.filter_map(byteRange => {
                     Editor.byteRangeToCharacterRange(byteRange, editor)
                   });

              let documentHighlightRanges =
                Feature_LanguageSupport.DocumentHighlights.getByLine(
                  ~bufferId,
                  ~line=EditorCoreTypes.LineNumber.toZeroBased(index),
                  languageSupport,
                );

              let highlights = searchHighlightRanges @ documentHighlightRanges;

              let shouldHighlight = i =>
                List.exists(
                  (r: CharacterRange.t) =>
                    CharacterIndex.toInt(r.start.character) <= i
                    && CharacterIndex.toInt(r.stop.character) >= i,
                  highlights,
                );

              // Draw error highlight
              switch (IntMap.find_opt(item, diagnostics)) {
              | Some(diags) =>
                let severity = Feature_Diagnostics.maxSeverity(diags);
                let color =
                  (
                    switch (severity) {
                    | Error => colors.errorForeground
                    | Warning => colors.warningForeground
                    | Info => colors.infoForeground
                    | Hint => colors.hintForeground
                    }
                  )
                  |> Revery.Color.multiplyAlpha(0.3);
                Skia.Paint.setColor(
                  minimapPaint,
                  Revery.Color.toSkia(color),
                );
                CanvasContext.drawRectLtwh(
                  ~left=0.,
                  ~top=rowHeight *. float(item) -. scrollY -. 1.0,
                  ~height=float(Constants.minimapCharacterHeight) +. 2.0,
                  ~width=float(width),
                  ~paint=minimapPaint,
                  canvasContext,
                );
              | None => ()
              };

              renderLine(
                ~scaleFactor,
                shouldHighlight,
                canvasContext,
                offset,
                tokens,
              );
            },
          (),
        );

        ImmediateList.render(
          ~scrollY,
          ~rowHeight,
          ~height=float(height),
          ~count,
          ~render=
            (item, offset) =>
              switch (IntMap.find_opt(item, diagnostics)) {
              | Some(v) =>
                List.iter(
                  (d: Diagnostic.t) =>
                    renderUnderline(
                      ~offset,
                      ~color=Revery.Color.rgba(1.0, 0., 0., 1.0),
                      d.range,
                    ),
                  v,
                )
              | None => ()
              },
          (),
        );

        Option.iter(
          EditorDiffMarkers.renderMinimap(
            ~editor,
            ~scrollY,
            ~rowHeight,
            ~x=Constants.leftMargin,
            ~height=float(height),
            ~width=2.,
            ~count,
            ~canvasContext,
            ~colors,
          ),
          diffMarkers,
        );
      }}
    />
    yankHighlightElement
  </View>;
};
