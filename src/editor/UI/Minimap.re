/*
 * Minimap.re
 *
 * Component that handles Minimap rendering
 */

open Revery.Draw;
open Revery.UI;

open Oni_Core;
module BufferViewTokenizer = Oni_Model.BufferViewTokenizer;
module Editor = Oni_Model.Editor;
module State = Oni_Model.State;

open Types;

let lineStyle = Style.[position(`Absolute), top(0)];

/* let rec getCurrentTokenColor = (tokens: list(TextmateClient.ColorizedToken.t), startPos: int, endPos: int) => { */
/*     switch (tokens) { */
/*     | [] => [TextmateClient.ColorizedToken.default] */
/*     | [last] => [last] */
/*     | [v1, v2, ...tail] when (v1.index <= startPos && v2.index > startPos) => [v1, v2, ...tail] */
/*     | [_, ...tail] => getCurrentTokenColor(tail, startPos, endPos) */
/*     } */
/* } */

let renderLine = (transform, yOffset, tokens: list(BufferViewTokenizer.t)) => {
  let f = (token: BufferViewTokenizer.t) => {
    switch (token.tokenType) {
    | Text =>
      let startPosition = Index.toZeroBasedInt(token.startPosition);
      let endPosition = Index.toZeroBasedInt(token.endPosition);
      let tokenWidth = endPosition - startPosition;

      /* let defaultForegroundColor: Color.t = theme.colors.editorForeground; */
      /* let defaultBackgroundColor: Color.t = theme.colors.editorBackground; */

      /* tokenCursor := getCurrentTokenColor(tokenCursor^, startPosition, endPosition); */
      /* let color: ColorizedToken.t = List.hd(tokenCursor^); */

      /* let foregroundColor = ColorMap.get(colorMap, color.foregroundColor, defaultForegroundColor, defaultBackgroundColor); */

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

          Mouse.setCapture(
            ~onMouseMove=
              evt =>
                if (isCaptured) {
                  let scrollTo = getScrollTo(evt.mouseY);
                  let minimapLineSize =
                    Constants.default.minimapCharacterWidth
                    + Constants.default.minimapCharacterHeight;
                  let linesInMinimap =
                    metrics.pixelHeight / minimapLineSize;
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
      GlobalContext.current().editorScroll(
        ~deltaY=scrollTo -. editor.scrollY -. float_of_int(linesInMinimap),
        (),
      );
      setActive(true);
    };

    ignore(width);

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
                  *. float_of_int(Editor.getTopVisibleLine(editor, metrics) - 1)
                  -. scrollY,
                ~height=rowHeight *. float_of_int(getMinimapSize(editor, metrics)),
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
          }}
        />
      </View>,
    );
  });
