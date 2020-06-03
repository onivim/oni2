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
      let startPosition = Index.toZeroBased(token.startPosition);
      let endPosition = Index.toZeroBased(token.endPosition);
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

let absoluteStyle =
  Style.[
    position(`Absolute),
    top(0),
    bottom(0),
    left(0),
    right(0),
    cursor(Revery.MouseCursors.pointer),
  ];

let getMinimapSize = (view: Editor.t) => {
  let currentViewSize = Editor.getVisibleView(view);

  view.viewLines < currentViewSize ? 0 : currentViewSize + 1;
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

  let%hook (maybeBbox, setBbox) = Hooks.state(None);

  let getScrollTo = (mouseY: float) => {
    maybeBbox
    |> Option.map(bbox => {
         let (_x, y, _width, _height) =
           Revery.Math.BoundingBox2d.getBounds(bbox);
         let totalHeight: int = Editor.getTotalHeightInPixels(editor);
         let visibleHeight: int = Editor.(editor.pixelHeight);
         let offsetMouseY = mouseY -. y;
         offsetMouseY /. float(visibleHeight) *. float(totalHeight);
       });
  };

  let scrollY = editor.minimapScrollY;

  let%hook (captureMouse, _state) =
    Hooks.mouseCapture(
      ~onMouseMove=
        ((), evt) => {
          evt.mouseY
          |> getScrollTo
          |> OptionEx.tap(scrollTo => {
               let minimapLineSize =
                 Constants.minimapLineSpacing
                 + Constants.minimapCharacterHeight;
               let linesInMinimap = editor.pixelHeight / minimapLineSize;
               dispatch(
                 Msg.MinimapDragged({
                   newPixelScrollY: scrollTo -. float(linesInMinimap),
                 }),
               );
             })
          |> Option.map(_ => ())
        },
      ~onMouseUp=(_, _) => None,
      (),
    );

  let onMouseDown = (evt: NodeEvents.mouseButtonEventParams) => {
    evt.mouseY
    |> getScrollTo
    |> Option.iter(scrollTo => {
         let minimapLineSize =
           Constants.minimapLineSpacing + Constants.minimapCharacterHeight;
         let linesInMinimap = editor.pixelHeight / minimapLineSize;
         if (evt.button == Revery.MouseButton.BUTTON_LEFT) {
           dispatch(
             Msg.MinimapClicked({
               newPixelScrollY: scrollTo -. float(linesInMinimap),
             }),
           );
           captureMouse();
         };
       });
  };

  <View
    style=absoluteStyle
    onMouseDown
    onBoundingBoxChanged={bbox => setBbox(_ => Some(bbox))}>
    <Canvas
      style=absoluteStyle
      render={canvasContext => {
        if (showSlider) {
          /* Draw slider/viewport */
          Skia.Paint.setColor(
            minimapPaint,
            Revery.Color.toSkia(colors.minimapSliderBackground),
          );
          CanvasContext.drawRectLtwh(
            ~left=0.,
            ~top=
              rowHeight
              *. float(Editor.getTopVisibleLine(editor) - 1)
              -. scrollY,
            ~height=rowHeight *. float(getMinimapSize(editor)),
            ~width=float(width),
            ~paint=minimapPaint,
            canvasContext,
          );
        };

        /* Draw cursor line */
        Skia.Paint.setColor(
          minimapPaint,
          Revery.Color.toSkia(colors.lineHighlightBackground),
        );
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
