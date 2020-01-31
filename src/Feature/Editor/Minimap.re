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
module Option = Utility.Option;

module Constants = {
  include Constants;

  let leftMargin = 2.;
  let diffMarkerWidth = 2.;
  let gutterMargin = 2.;
  let gutterWidth = diffMarkerWidth +. gutterMargin;
};

let lineStyle = Style.[position(`Absolute), top(0)];

let renderLine =
    (
      shouldHighlight,
      transform,
      yOffset,
      tokens: list(BufferViewTokenizer.t),
    ) => {
  let f = (token: BufferViewTokenizer.t) => {
    switch (token.tokenType) {
    | Text =>
      let startPosition = Index.toZeroBased(token.startPosition);
      let endPosition = Index.toZeroBased(token.endPosition);
      let tokenWidth = endPosition - startPosition;

      let x = float(Constants.default.minimapCharacterWidth * startPosition);
      let height = float(Constants.default.minimapCharacterHeight);
      let width = float(tokenWidth * Constants.default.minimapCharacterWidth);

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

      Shapes.drawRect(~transform, ~y, ~x, ~color, ~width, ~height, ());
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
    float(
      Constants.default.minimapCharacterHeight
      + Constants.default.minimapLineSpacing,
    );

  let%hook (mouseState, dispatch) =
    React.Hooks.reducer(~initialState, reducer);

  let getScrollTo = (mouseY: float) => {
    let totalHeight: int = Editor.getTotalSizeInPixels(editor, metrics);
    let visibleHeight: int = metrics.pixelHeight;
    let offsetMouseY: int =
      int_of_float(mouseY) - Constants.default.tabHeight;
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
      Constants.default.minimapLineSpacing
      + Constants.default.minimapCharacterHeight;
    let linesInMinimap = metrics.pixelHeight / minimapLineSize;
    if (evt.button == Revery_Core.MouseButton.BUTTON_LEFT) {
      onScroll(scrollTo -. editor.scrollY -. float(linesInMinimap));

      Mouse.setCapture(
        ~onMouseMove=
          evt => {
            let scrollTo = getScrollTo(evt.mouseY);
            let minimapLineSize =
              Constants.default.minimapLineSpacing
              + Constants.default.minimapCharacterHeight;
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
    <OpenGL
      style=absoluteStyle
      render={(transform, _) => {
        if (showSlider) {
          /* Draw slider/viewport */
          Shapes.drawRect(
            ~transform,
            ~x=0.,
            ~y=
              rowHeight
              *. float(Editor.getTopVisibleLine(editor, metrics) - 1)
              -. scrollY,
            ~height=rowHeight *. float(getMinimapSize(editor, metrics)),
            ~width=float(width),
            ~color=theme.scrollbarSliderHoverBackground,
            (),
          );
        };

        let cursorPosition = Editor.getPrimaryCursor(editor);
        /* Draw cursor line */
        Shapes.drawRect(
          ~transform,
          ~x=Constants.leftMargin,
          ~y=
            rowHeight
            *. float(Index.toZeroBased(cursorPosition.line))
            -. scrollY,
          ~height=float(Constants.default.minimapCharacterHeight),
          ~width=float(width),
          ~color=theme.editorLineHighlightBackground,
          (),
        );

        let renderRange = (~color, ~offset, range: Range.t) =>
          {let startX =
             float(Index.toZeroBased(range.start.column))
             *. float(Constants.default.minimapCharacterWidth)
             +. Constants.leftMargin
             +. Constants.gutterWidth;
           let endX =
             float(Index.toZeroBased(range.stop.column))
             *. float(Constants.default.minimapCharacterWidth);

           Shapes.drawRect(
             ~transform,
             ~x=startX -. 1.0,
             ~y=offset -. 1.0,
             ~height=float(Constants.default.minimapCharacterHeight) +. 2.0,
             ~width=endX -. startX +. 2.,
             ~color,
             (),
           )};

        let renderUnderline = (~color, ~offset, range: Range.t) =>
          {let startX =
             float(Index.toZeroBased(range.start.column))
             *. float(Constants.default.minimapCharacterWidth)
             +. Constants.leftMargin
             +. Constants.gutterWidth;
           let endX =
             float(Index.toZeroBased(range.stop.column))
             *. float(Constants.default.minimapCharacterWidth);

           Shapes.drawRect(
             ~transform,
             ~x=startX -. 1.0,
             ~y=offset +. float(Constants.default.minimapCharacterHeight),
             ~height=1.0,
             ~width=endX -. startX +. 2.,
             ~color,
             (),
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
                Shapes.drawRect(
                  ~transform,
                  ~x=0.,
                  ~y=rowHeight *. float(item) -. scrollY -. 1.0,
                  ~height=
                    float(Constants.default.minimapCharacterHeight) +. 2.0,
                  ~width=float(width),
                  ~color=Color.rgba(1.0, 0.0, 0.0, 0.3),
                  (),
                )
              | None => ()
              };

              renderLine(shouldHighlight, transform, offset, tokens);
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
            ~transform,
            ~theme,
          ),
          diffMarkers,
        );
      }}
    />
  </View>;
};
