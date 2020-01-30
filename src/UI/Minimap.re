/*
 * Minimap.re
 *
 * Component that handles Minimap rendering
 */

open EditorCoreTypes;
open Revery;
open Revery.Draw;
open Revery.UI;

open Oni_Core;
module BufferHighlights = Oni_Model.BufferHighlights;
module BufferViewTokenizer = Oni_Model.BufferViewTokenizer;
module Diagnostic = Oni_Model.Diagnostic;
module Editor = Oni_Model.Editor;
module Selectors = Oni_Model.Selectors;
module State = Oni_Model.State;

let lineStyle = Style.[position(`Absolute), top(0)];

let minimapPaint = Skia.Paint.make();

let drawRect = (~x, ~y, ~width, ~height, ~color, canvasContext) => {
  Skia.Paint.setColor(minimapPaint, Color.toSkia(color));
  let rect = Skia.Rect.makeLtrb(x, y, x +. width, y +. height);
  CanvasContext.drawRect(~rect, ~paint=minimapPaint, canvasContext);
};

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

      let x =
        float_of_int(Constants.default.minimapCharacterWidth * startPosition);
      let height = float_of_int(Constants.default.minimapCharacterHeight);
      let width =
        float_of_int(tokenWidth * Constants.default.minimapCharacterWidth);

      let emphasis = shouldHighlight(startPosition);
      let color =
        emphasis ? token.color : Color.multiplyAlpha(0.5, token.color);

      let offset = 1.0;
      let halfOffset = offset /. 2.0;

      let x = emphasis ? x -. halfOffset : x;
      let y = yOffset;
      let width = emphasis ? width +. offset : width;

      drawRect(~x, ~y, ~width, ~height, ~color, canvasContext);
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
                ~state: State.t,
                ~editor: Editor.t,
                ~width: int,
                ~height: int,
                ~count,
                ~diagnostics,
                ~getTokensForLine: int => list(BufferViewTokenizer.t),
                ~selection: Hashtbl.t(Index.t, list(Range.t)),
                ~metrics,
                (),
              ) => {
  let rowHeight =
    float_of_int(
      Constants.default.minimapCharacterHeight
      + Constants.default.minimapLineSpacing,
    );

  let editorId = Editor.getId(editor);
  let%hook (mouseState, dispatch) =
    React.Hooks.reducer(~initialState, reducer);

  let getScrollTo = (mouseY: float) => {
    let totalHeight: int = Editor.getTotalSizeInPixels(editor, metrics);
    let visibleHeight: int = metrics.pixelHeight;
    let offsetMouseY: int =
      int_of_float(mouseY) - Constants.default.tabHeight;
    float_of_int(offsetMouseY)
    /. float_of_int(visibleHeight)
    *. float_of_int(totalHeight);
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
      Constants.default.minimapLineSpacing
      + Constants.default.minimapCharacterHeight;
    let linesInMinimap = metrics.pixelHeight / minimapLineSize;
    if (evt.button == Revery_Core.MouseButton.BUTTON_LEFT) {
      GlobalContext.current().editorScrollDelta(
        ~editorId,
        ~deltaY=scrollTo -. editor.scrollY -. float_of_int(linesInMinimap),
        (),
      );
      Mouse.setCapture(
        ~onMouseMove=
          evt => {
            let scrollTo = getScrollTo(evt.mouseY);
            let minimapLineSize =
              Constants.default.minimapLineSpacing
              + Constants.default.minimapCharacterHeight;
            let linesInMinimap = metrics.pixelHeight / minimapLineSize;
            let scrollTo = scrollTo -. float_of_int(linesInMinimap);
            GlobalContext.current().editorSetScroll(
              ~editorId,
              ~scrollY=scrollTo,
              (),
            );
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
        if (Configuration.getValue(
              c => c.editorMinimapShowSlider,
              state.configuration,
            )) {
          /* Draw current view */
          drawRect(
            ~x=0.,
            ~y=
              rowHeight
              *. float_of_int(Editor.getTopVisibleLine(editor, metrics) - 1)
              -. scrollY,
            ~height=
              rowHeight *. float_of_int(getMinimapSize(editor, metrics)),
            ~width=float_of_int(width),
            ~color=state.theme.scrollbarSliderHoverBackground,
            canvasContext,
          );
        };

        let cursorPosition = Editor.getPrimaryCursor(editor);
        /* Draw cursor line */
        drawRect(
          ~x=0.,
          ~y=
            rowHeight
            *. float_of_int(Index.toZeroBased(cursorPosition.line))
            -. scrollY,
          ~height=float_of_int(Constants.default.minimapCharacterHeight),
          ~width=float_of_int(width),
          ~color=state.theme.editorLineHighlightBackground,
          canvasContext,
        );

        let renderRange = (~color, ~offset, range: Range.t) =>
          {let startX =
             Index.toZeroBased(range.start.column)
             * Constants.default.minimapCharacterWidth
             |> float_of_int;
           let endX =
             Index.toZeroBased(range.stop.column)
             * Constants.default.minimapCharacterWidth
             |> float_of_int;

           drawRect(
             ~x=startX -. 1.0,
             ~y=offset -. 1.0,
             ~height=
               float_of_int(Constants.default.minimapCharacterHeight) +. 2.0,
             ~width=endX -. startX +. 2.,
             ~color,
             canvasContext,
           )};

        let renderUnderline = (~color, ~offset, range: Range.t) =>
          {let startX =
             Index.toZeroBased(range.start.column)
             * Constants.default.minimapCharacterWidth
             |> float_of_int;
           let endX =
             Index.toZeroBased(range.stop.column)
             * Constants.default.minimapCharacterWidth
             |> float_of_int;

           drawRect(
             ~x=startX -. 1.0,
             ~y=
               offset
               +. float_of_int(Constants.default.minimapCharacterHeight),
             ~height=1.0,
             ~width=endX -. startX +. 2.,
             ~color,
             canvasContext,
           )};

        ImmediateList.render(
          ~scrollY,
          ~rowHeight,
          ~height=float_of_int(height),
          ~count,
          ~render=
            (item, offset) => {
              open Range;
              /* draw selection */
              let index = Index.fromZeroBased(item);
              switch (Hashtbl.find_opt(selection, index)) {
              | None => ()
              | Some(v) =>
                let selectionColor = state.theme.editorSelectionBackground;
                List.iter(renderRange(~color=selectionColor, ~offset), v);
              };

              let tokens = getTokensForLine(item);

              let highlightRanges =
                BufferHighlights.getHighlightsByLine(
                  ~bufferId=editor.bufferId,
                  ~line=index,
                  state.bufferHighlights,
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
                drawRect(
                  ~x=0.,
                  ~y=rowHeight *. float_of_int(item) -. scrollY -. 1.0,
                  ~height=
                    float_of_int(Constants.default.minimapCharacterHeight)
                    +. 2.0,
                  ~width=float_of_int(width),
                  ~color=Color.rgba(1.0, 0.0, 0.0, 0.3),
                  canvasContext,
                )
              | None => ()
              };

              renderLine(shouldHighlight, canvasContext, offset, tokens);
            },
          (),
        );

        ImmediateList.render(
          ~scrollY,
          ~rowHeight,
          ~height=float_of_int(height),
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
      }}
    />
  </View>;
};
