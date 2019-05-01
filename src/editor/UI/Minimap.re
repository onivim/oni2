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
module State = Oni_Model.State;

open Types;

let lineStyle = Style.[position(`Absolute), top(0)];

let renderLine = (transform, yOffset, tokens: list(BufferViewTokenizer.t)) => {
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

      Shapes.drawRect(
        ~transform,
        ~y=yOffset,
        ~x,
        ~color=token.color,
        ~width,
        ~height,
        (),
      );
    | _ => ()
    };
  };

  List.iter(f, tokens);
};

let component = React.component("Minimap");

let absoluteStyle =
  Style.[position(`Absolute), top(0), bottom(0), left(0), right(0)];

let getMinimapSize = (view: Editor.t, metrics) => {
  let currentViewSize = Editor.getVisibleView(metrics);

  view.viewLines < currentViewSize ? 0 : currentViewSize + 1;
};

let createElement =
    (
      ~state: State.t,
      ~editor: Editor.t,
      ~width: int,
      ~height: int,
      ~count,
      ~diagnostics,
      ~getTokensForLine: int => list(BufferViewTokenizer.t),
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

    let (isActive, setActive, hooks) = React.Hooks.state(false, hooks);

    let getScrollTo = (mouseY: float) => {
      let totalHeight: int = Editor.getTotalSizeInPixels(editor, metrics);
      let visibleHeight: int = metrics.pixelHeight;
      let offsetMouseY: int = int_of_float(mouseY) - Tab.tabHeight;
      float_of_int(offsetMouseY)
      /. float_of_int(visibleHeight)
      *. float_of_int(totalHeight);
    };

    let scrollComplete = () => {
      setActive(false);
    };

    let hooks =
      React.Hooks.effect(
        Always,
        () => {
          let isCaptured = isActive;
          let startPosition = editor.scrollY;
          if (isCaptured) {
            Mouse.setCapture(
              ~onMouseMove=
                evt => {
                  let scrollTo = getScrollTo(evt.mouseY);
                  let minimapLineSize =
                    Constants.default.minimapCharacterWidth
                    + Constants.default.minimapCharacterHeight;
                  let linesInMinimap = metrics.pixelHeight / minimapLineSize;
                  GlobalContext.current().editorScroll(
                    ~deltaY=
                      (startPosition -. scrollTo)
                      *. (-1.)
                      -. float_of_int(linesInMinimap),
                    (),
                  );
                },
              ~onMouseUp=_evt => scrollComplete(),
              (),
            );
          };
          Some(
            () =>
              if (isCaptured) {
                Mouse.releaseCapture();
              },
          );
        },
        hooks,
      );

    let scrollY = editor.minimapScrollY;

    let onMouseDown = (evt: NodeEvents.mouseButtonEventParams) => {
      let scrollTo = getScrollTo(evt.mouseY);
      let minimapLineSize =
        Constants.default.minimapCharacterWidth
        + Constants.default.minimapCharacterHeight;
      let linesInMinimap = metrics.pixelHeight / minimapLineSize;
      if (evt.button == Revery_Core.MouseButton.BUTTON_LEFT) {
        GlobalContext.current().editorScroll(
          ~deltaY=scrollTo -. editor.scrollY -. float_of_int(linesInMinimap),
          (),
        );
        setActive(true);
      };
    };

    (
      hooks,
      <View style=absoluteStyle onMouseDown>
        <OpenGL
          style=absoluteStyle
          render={(transform, _) => {
            if (state.configuration.editorMinimapShowSlider) {
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
                ~color=state.theme.colors.scrollbarSliderHoverBackground,
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
              ~color=state.theme.colors.editorLineHighlightBackground,
              (),
            );

            FlatList.render(
              ~scrollY,
              ~rowHeight,
              ~height=float_of_int(height),
              ~count,
              ~render=
                (item, offset) => {
                  let tokens = getTokensForLine(item);
                  renderLine(transform, offset, tokens);
                },
              (),
            );

            FlatList.render(
              ~scrollY,
              ~rowHeight,
              ~height=float_of_int(height),
              ~count,
              ~render=
                (item, offset) => {
                  let renderDiagnostics = (d: Diagnostics.Diagnostic.t) =>
                    {let startX =
                       Index.toZeroBasedInt(d.range.startPosition.character)
                       * Constants.default.minimapCharacterWidth
                       |> float_of_int
                     let endX =
                       Index.toZeroBasedInt(d.range.endPosition.character)
                       * Constants.default.minimapCharacterWidth
                       |> float_of_int

                     Shapes.drawRect(
                       ~transform,
                       ~x=startX -. 1.0,
                       ~y=offset -. 1.0,
                       ~height=
                         float_of_int(
                           Constants.default.minimapCharacterHeight,
                         )
                         +. 2.0,
                       ~width=endX -. startX +. 2.,
                       ~color=Color.rgba(1.0, 0., 0., 0.7),
                       (),
                     )};

                  switch (IntMap.find_opt(item, diagnostics)) {
                  | Some(v) => List.iter(renderDiagnostics, v)
                  | None => ()
                  };
                },
              (),
            );
          }}
        />
      </View>,
    );
  });
