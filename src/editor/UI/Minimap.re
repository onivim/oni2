/*
 * Minimap.re
 *
 * Component that handles Minimap rendering
 */

open Revery.Draw;
open Revery.UI;

open Oni_Core;

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

let renderLine = (transform, yOffset, tokens: list(Tokenizer.t)) => {
  let f = (token: Tokenizer.t) => {
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
      ~y=float_of_int(yOffset),
      ~x,
      ~color=token.color,
      ~width,
      ~height,
      (),
    );
  };

  List.iter(f, tokens);
};

let component = React.component("Minimap");

let absoluteStyle =
  Style.[position(`Absolute), top(0), bottom(0), left(0), right(0)];

let createElement =
    (
      ~state: State.t,
      ~width: int,
      ~height: int,
      ~count,
      ~getTokensForLine: int => list(Tokenizer.t),
      ~children as _,
      (),
    ) =>
  component(hooks => {
    let rowHeight =
      Constants.default.minimapCharacterHeight
      + Constants.default.minimapLineSpacing;

    let scrollY = state.editor.minimapScrollY;

    ignore(width);

    (
      hooks,
      <View style=absoluteStyle>
        <OpenGL
          style=absoluteStyle
          onMouseOver={_ => Console.log("Entered minimap....")}
          onMouseOut={_ => Console.log("Left minimap....")}
          render={(transform, _) => {

            /* Draw current view */
            Shapes.drawRect(
              ~transform,
              ~x=0.,
              ~y=
                float_of_int(
                  rowHeight
                  * Editor.getTopVisibleLine(state.editor)
                  - scrollY,
                ),
              ~height=float_of_int(rowHeight * Editor.getVisibleView(state.editor)),
              ~width=float_of_int(width),
              ~color=state.theme.colors.minimapHighlightBackground,
              (),
            );

            /* Draw cursor line */
            Shapes.drawRect(
              ~transform,
              ~x=0.,
              ~y=
                float_of_int(
                  rowHeight
                  * Index.toZeroBasedInt(state.editor.cursorPosition.line)
                  - scrollY,
                ),
              ~height=float_of_int(Constants.default.minimapCharacterHeight),
              ~width=float_of_int(width),
              ~color=state.theme.colors.editorLineHighlightBackground,
              (),
            );

            FlatList.render(
              ~scrollY,
              ~rowHeight,
              ~height,
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
