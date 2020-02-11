/*
 * Minimap.re
 *
 * Component that handles Minimap rendering
 */

open EditorCoreTypes;
open Oni_Core;
open Revery;
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
        emphasis ? token.color : Color.multiplyAlpha(0.5, token.color);

      let offset = 1.0;
      let halfOffset = offset /. 2.0;

      let x =
        (emphasis ? x -. halfOffset : x)
        +. Constants.leftMargin
        +. Constants.gutterWidth;
      let y = yOffset;
      let width = emphasis ? width +. offset : width;

      Skia.Paint.setColor(minimapPaint, Color.toSkia(color));
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
    cursor(MouseCursors.pointer),
  ];

let getMinimapSize = (view: Editor.t, metrics) => {
  let currentViewSize = Editor.getVisibleView(metrics);

  view.viewLines < currentViewSize ? 0 : currentViewSize + 1;
};

type mouseCaptureState = {isCapturing: bool};

type action =
  | IsCapturing(bool);

let reducer = (action, _state) =>
  switch (action) {
  | IsCapturing(isCapturing) => {isCapturing: isCapturing}
  };

let initialState = {isCapturing: false};

let%component make =
              (
                ~editor: Editor.t,
                ~width: int,
                ~height: int,
                ~count,
                ~diagnostics,
                ~getTokensForLine: int => list(BufferViewTokenizer.t),
                ~selection: Hashtbl.t(Index.t, list(Range.t)),
                ~metrics,
                ~onScroll,
                ~showSlider,
                ~theme: Theme.t,
                ~bufferHighlights,
                ~diffMarkers,
                (),
              ) => {
  let rowHeight =
    float(Constants.minimapCharacterHeight + Constants.minimapLineSpacing);

  let%hook (mouseState, dispatch) =
    React.Hooks.reducer(~initialState, reducer);

  let getScrollTo = (mouseY: float) => {
    let totalHeight: int = Editor.getTotalSizeInPixels(editor, metrics);
    let visibleHeight: int = metrics.pixelHeight;
    let offsetMouseY: int = int_of_float(mouseY) - Constants.tabHeight;
    float(offsetMouseY) /. float(visibleHeight) *. float(totalHeight);
  };

  let scrollComplete = () => {
    Mouse.releaseCapture();
    dispatch(IsCapturing(false));
  };

  let%hook () =
    React.Hooks.effect(OnMount, () => {
      Some(
        () =>
          if (mouseState.isCapturing) {
            Mouse.releaseCapture();
          },
      )
    });

  let scrollY = editor.minimapScrollY;

  let onMouseDown = (evt: NodeEvents.mouseButtonEventParams) => {
    let scrollTo = getScrollTo(evt.mouseY);
    let minimapLineSize =
      Constants.minimapLineSpacing + Constants.minimapCharacterHeight;
    let linesInMinimap = metrics.pixelHeight / minimapLineSize;
    if (evt.button == Revery_Core.MouseButton.BUTTON_LEFT) {
      onScroll(scrollTo -. editor.scrollY -. float(linesInMinimap));

      Mouse.setCapture(
        ~onMouseMove=
          evt => {
            let scrollTo = getScrollTo(evt.mouseY);
            let minimapLineSize =
              Constants.minimapLineSpacing + Constants.minimapCharacterHeight;
            let linesInMinimap = metrics.pixelHeight / minimapLineSize;
            onScroll(scrollTo -. float(linesInMinimap));
          },
        ~onMouseUp=_evt => {scrollComplete()},
        (),
      );
      dispatch(IsCapturing(true));
    };
  };

  <View style=absoluteStyle onMouseDown>
    <Canvas
      style=absoluteStyle
      render={canvasContext => {
        if (showSlider) {
          /* Draw slider/viewport */
          Skia.Paint.setColor(
            minimapPaint,
            Color.toSkia(theme.scrollbarSliderHoverBackground),
          );
          CanvasContext.drawRectLtwh(
            ~left=0.,
            ~top=
              rowHeight
              *. float(Editor.getTopVisibleLine(editor, metrics) - 1)
              -. scrollY,
            ~height=rowHeight *. float(getMinimapSize(editor, metrics)),
            ~width=float(width),
            ~paint=minimapPaint,
            canvasContext,
          );
        };

        let cursorPosition = Editor.getPrimaryCursor(editor);
        /* Draw cursor line */
        Skia.Paint.setColor(
          minimapPaint,
          Color.toSkia(theme.editorLineHighlightBackground),
        );
        CanvasContext.drawRectLtwh(
          ~left=Constants.leftMargin,
          ~top=
            rowHeight
            *. float(Index.toZeroBased(cursorPosition.line))
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

           Skia.Paint.setColor(minimapPaint, Color.toSkia(color));
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

           Skia.Paint.setColor(minimapPaint, Color.toSkia(color));
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
                let selectionColor = theme.editorSelectionBackground;
                List.iter(renderRange(~color=selectionColor, ~offset), v);
              };

              let tokens = getTokensForLine(item);

              let highlightRanges =
                BufferHighlights.getHighlightsByLine(
                  ~bufferId=editor.bufferId,
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
                let color = Color.rgba(1.0, 0.0, 0.0, 0.3);
                Skia.Paint.setColor(minimapPaint, Color.toSkia(color));
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
                      ~color=Color.rgba(1.0, 0., 0., 1.0),
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
            ~theme,
          ),
          diffMarkers,
        );
      }}
    />
  </View>;
};
