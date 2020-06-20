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

module BufferHighlights = Oni_Syntax.BufferHighlights;
module Diagnostic = Feature_LanguageSupport.Diagnostic;

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
      shouldHighlight,
      canvasContext,
      yOffset,
      tokens: list(BufferViewTokenizer.t),
    ) => {
  let f = (token: BufferViewTokenizer.t) => {
    switch (token.tokenType) {
    | Text =>
      // TODO: Fix this
      let startPosition = Index.toZeroBased(token.startIndex);
      let endPosition = Index.toZeroBased(token.endIndex);
      let tokenWidth = endPosition - startPosition;

      let x = float(Constants.minimapCharacterWidth * startPosition);
      let height = float(Constants.minimapCharacterHeight);
      let width = float(tokenWidth * Constants.minimapCharacterWidth);

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
                ~cursorPosition: Location.t,
                ~width: int,
                ~height: int,
                ~count,
                ~diagnostics,
                ~getTokensForLine: int => list(BufferViewTokenizer.t),
                ~selection: Hashtbl.t(Index.t, list(Range.t)),
                ~showSlider,
                ~colors: Colors.t,
                ~bufferHighlights,
                ~diffMarkers,
                (),
              ) => {
  let rowHeight =
    float(Constants.minimapCharacterHeight + Constants.minimapLineSpacing);

  let scrollY = Editor.minimapScrollY(editor);

  let thumbTop =
    rowHeight *. float(Editor.getTopVisibleLine(editor) - 1) -. scrollY;
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

  <View
    style={Styles.container(backgroundColor)}
    onMouseDown
    onMouseMove
    onMouseOver
    onMouseLeave
    onBoundingBoxChanged={bbox => setBbox(_ => Some(bbox))}>
    <Canvas
      style=Styles.absolute
      render={canvasContext => {
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
        CanvasContext.drawRectLtwh(
          ~left=Constants.leftMargin,
          ~top=
            rowHeight
            *. float(Index.toZeroBased(Location.(cursorPosition.line)))
            -. scrollY,
          ~height=float(Constants.minimapCharacterHeight),
          ~width=float(width),
          ~paint=minimapPaint,
          canvasContext,
        );

        let renderRange = (~color, ~offset, range: Range.t) =>
          {let startX =
             float(Index.toZeroBased(range.start.column))
             *. float(Constants.minimapCharacterWidth)
             +. Constants.leftMargin
             +. Constants.gutterWidth;
           let endX =
             float(Index.toZeroBased(range.stop.column))
             *. float(Constants.minimapCharacterWidth);

           Skia.Paint.setColor(minimapPaint, Revery.Color.toSkia(color));
           CanvasContext.drawRectLtwh(
             ~left=startX -. 1.0,
             ~top=offset -. 1.0,
             ~height=float(Constants.minimapCharacterHeight) +. 2.0,
             ~width=endX -. startX +. 2.,
             ~paint=minimapPaint,
             canvasContext,
           )};

        let renderUnderline = (~color, ~offset, range: Range.t) =>
          {let startX =
             float(Index.toZeroBased(range.start.column))
             *. float(Constants.minimapCharacterWidth)
             +. Constants.leftMargin
             +. Constants.gutterWidth;
           let endX =
             float(Index.toZeroBased(range.stop.column))
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

        ImmediateList.render(
          ~scrollY,
          ~rowHeight,
          ~height=float(height),
          ~count,
          ~render=
            (item, offset) => {
              open Range;
              /* draw selection */
              let index = Index.fromZeroBased(item);
              switch (Hashtbl.find_opt(selection, index)) {
              | None => ()
              | Some(v) =>
                let color = colors.minimapSelectionHighlight;
                List.iter(renderRange(~color, ~offset), v);
              };

              let tokens = getTokensForLine(item);

              let highlightRanges =
                BufferHighlights.getHighlightsByLine(
                  ~bufferId=Editor.getBufferId(editor),
                  ~line=index,
                  bufferHighlights,
                );

              let shouldHighlight = i =>
                List.exists(
                  r =>
                    Index.toZeroBased(r.start.column) <= i
                    && Index.toZeroBased(r.stop.column) >= i,
                  highlightRanges,
                );

              // Draw error highlight
              switch (IntMap.find_opt(item, diagnostics)) {
              | Some(_) =>
                let color = Revery.Color.rgba(1.0, 0.0, 0.0, 0.3);
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

              renderLine(shouldHighlight, canvasContext, offset, tokens);
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
          EditorDiffMarkers.render(
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
  </View>;
};
