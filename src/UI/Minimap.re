/*
 * Minimap.re
 *
 * Component that handles Minimap rendering
 */

open Revery;
open Revery.Draw;
open Revery.UI;

open Oni_Core;
module BufferViewTokenizer = Oni_Model.BufferViewTokenizer;
module Diagnostics = Oni_Model.Diagnostics;
module Editor = Oni_Model.Editor;
module Selectors = Oni_Model.Selectors;
module State = Oni_Model.State;

open Types;

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
      let startPosition = Index.toZeroBasedInt(token.startPosition);
      let endPosition = Index.toZeroBasedInt(token.endPosition);
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

      Shapes.drawRect(~transform, ~y, ~x, ~color, ~width, ~height, ());
    | _ => ()
    };
  };

  List.iter(f, tokens);
};

let absoluteStyle =
  Style.[position(`Absolute), top(0), bottom(0), left(0), right(0), cursor(MouseCursors.pointer)];

let getMinimapSize = (view: Editor.t, metrics) => {
  let currentViewSize = Editor.getVisibleView(metrics);

  view.viewLines < currentViewSize ? 0 : currentViewSize + 1;
};

type mouseCaptureState = {
  shouldCapture: bool,
  isCapturing: bool,
};

type action = 
| StartCapture
| EndCapture
| IsCapturing(bool);

let reducer = (action, state) => switch(action) {
| StartCapture => { ...state, shouldCapture: true }
| EndCapture => { ...state, shouldCapture: false }
| IsCapturing(isCapturing) => { ...state, isCapturing }
}

let initialState = { shouldCapture: false, isCapturing: false };

let component = React.component("Minimap");

let createElement =
    (
      ~state: State.t,
      ~editor: Editor.t,
      ~width: int,
      ~height: int,
      ~count,
      ~diagnostics,
      ~getTokensForLine: int => list(BufferViewTokenizer.t),
      ~selection: Hashtbl.t(int, list(Range.t)),
      ~metrics,
      ~children as _,
      (),
    ) =>
  component(hooks => {
    let rowHeight =
      float_of_int(
        Constants.default.minimapCharacterHeight
        + Constants.default.minimapLineSpacing,
      );

    let (mouseState, dispatch, hooks) = React.Hooks.reducer(initialState, reducer, hooks);

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

    let string_of_bool = (v) => v ? "true" : "false";

    let hooks =
      React.Hooks.effect(
        OnMount,
        () => {
          Some(
            () => {
              if (mouseState.isCapturing) {
                Mouse.releaseCapture();
              }
            }
          );
        },
        hooks,
      );

    let scrollY = editor.minimapScrollY;

    let onMouseDown = (evt: NodeEvents.mouseButtonEventParams) => {
      Log.info("ONMOUSEDOWN");
      let scrollTo = getScrollTo(evt.mouseY);
      let minimapLineSize =
        Constants.default.minimapLineSpacing
        + Constants.default.minimapCharacterHeight;
      let linesInMinimap = metrics.pixelHeight / minimapLineSize;
      if (evt.button == Revery_Core.MouseButton.BUTTON_LEFT) {
        GlobalContext.current().editorScroll(
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
                  /*GlobalContext.current().editorScroll(
                    ~deltaY=
                      (editor.scrollY -. scrollTo)
                      *. (-1.)
                      -. float_of_int(linesInMinimap),
                    (),
                  );*/
                  GlobalContext.current().editorScroll2(
                    ~scrollY=scrollTo,
                    (),
                  );
                },
              ~onMouseUp=_evt => {
                Log.info("onMouseUp");
                scrollComplete();
              },
              ~onMouseLeaveWindow=() => {
                Log.info("onMouseLeaveWindow");
                scrollComplete();
              },
              (),
            );
            dispatch(IsCapturing(true));
      };
    };

    (
      hooks,
      <View style=absoluteStyle onMouseDown>
        <OpenGL
          style=absoluteStyle
          render={(transform, _) => {
            if (Configuration.getValue(
                  c => c.editorMinimapShowSlider,
                  state.configuration,
                )) {
              /* Draw current view */
              Shapes.drawRect(
                ~transform,
                ~x=0.,
                ~y=
                  rowHeight
                  *. float_of_int(
                       Editor.getTopVisibleLine(editor, metrics) - 1,
                     )
                  -. scrollY,
                ~height=
                  rowHeight *. float_of_int(getMinimapSize(editor, metrics)),
                ~width=float_of_int(width),
                ~color=state.theme.scrollbarSliderHoverBackground,
                (),
              );
            };
            /* Draw cursor line */
            Shapes.drawRect(
              ~transform,
              ~x=0.,
              ~y=
                rowHeight
                *. float_of_int(
                     Index.toZeroBasedInt(editor.cursorPosition.line),
                   )
                -. scrollY,
              ~height=float_of_int(Constants.default.minimapCharacterHeight),
              ~width=float_of_int(width),
              ~color=state.theme.editorLineHighlightBackground,
              (),
            );

            let searchHighlights =
              Selectors.getSearchHighlights(state, editor.bufferId);

            let renderRange = (~color, ~offset, range: Range.t) =>
              {let startX =
                 Index.toZeroBasedInt(range.startPosition.character)
                 * Constants.default.minimapCharacterWidth
                 |> float_of_int;
               let endX =
                 Index.toZeroBasedInt(range.endPosition.character)
                 * Constants.default.minimapCharacterWidth
                 |> float_of_int;

               Shapes.drawRect(
                 ~transform,
                 ~x=startX -. 1.0,
                 ~y=offset -. 1.0,
                 ~height=
                   float_of_int(Constants.default.minimapCharacterHeight)
                   +. 2.0,
                 ~width=endX -. startX +. 2.,
                 ~color,
                 (),
               )};

            FlatList.render(
              ~scrollY,
              ~rowHeight,
              ~height=float_of_int(height),
              ~count,
              ~render=
                (item, offset) => {
                  open Range;
                  /* draw selection */
                  switch (Hashtbl.find_opt(selection, item)) {
                  | None => ()
                  | Some(v) =>
                    let selectionColor = state.theme.editorSelectionBackground;
                    List.iter(
                      renderRange(~color=selectionColor, ~offset),
                      v,
                    );
                  };

                  let tokens = getTokensForLine(item);
                  let highlightRanges =
                    switch (IntMap.find_opt(item, searchHighlights)) {
                    | Some(v) => v
                    | None => []
                    };
                  let shouldHighlight = i =>
                    List.exists(
                      r =>
                        Index.toInt0(r.startPosition.character) <= i
                        && Index.toInt0(r.endPosition.character) >= i,
                      highlightRanges,
                    );
                  renderLine(shouldHighlight, transform, offset, tokens);
                },
              (),
            );

            FlatList.render(
              ~scrollY,
              ~rowHeight,
              ~height=float_of_int(height),
              ~count,
              ~render=
                (item, offset) =>
                  switch (IntMap.find_opt(item, diagnostics)) {
                  | Some(v) =>
                    List.iter(
                      (d: Diagnostics.Diagnostic.t) =>
                        renderRange(
                          ~offset,
                          ~color=Color.rgba(1.0, 0., 0., 0.7),
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
      </View>,
    );
  });
