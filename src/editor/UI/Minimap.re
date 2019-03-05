/*
 * Minimap.re
 *
 * Component that handles Minimap rendering
 */

/* open Reglfw.Glfw; */
open Revery;
open Revery.Draw;
open Revery.UI;

open Oni_Core;
/* open Oni_Core.Theme; */
/* open Oni_Core.Theme.EditorColors; */
open Oni_Core.TextmateClient;
open Oni_Core.TextmateClient.ColorizedToken;

open Types;

let lineStyle = Style.[position(`Absolute), top(0)];

let rec getCurrentTokenColor = (tokens: list(TextmateClient.ColorizedToken.t), startPos: int, endPos: int) => {
    switch (tokens) {
    | [] => [TextmateClient.ColorizedToken.default]
    | [last] => [last]
    | [v1, v2, ...tail] when (v1.index <= startPos && v2.index > startPos) => [v1, v2, ...tail]
    | [_, ...tail] => getCurrentTokenColor(tail, startPos, endPos)
    }
}

let tokensToElement = (theme: Oni_Core.Theme.t, transform, yOffset, tokens: list(Tokenizer.t), tokenColors: list(ColorizedToken.t), colorMap) => {

  let tokenCursor = ref(tokenColors);

  let f = (token: Tokenizer.t) => {
      let startPosition = Index.toZeroBasedInt(token.startPosition);
      let endPosition = Index.toZeroBasedInt(token.endPosition);
    let tokenWidth =
        endPosition
      - startPosition;

    let defaultForegroundColor: Color.t = theme.colors.editorForeground;
    let defaultBackgroundColor: Color.t = theme.colors.editorBackground;

    tokenCursor := getCurrentTokenColor(tokenCursor^, startPosition, endPosition);
    let color: ColorizedToken.t = List.hd(tokenCursor^);

    let foregroundColor = ColorMap.get(colorMap, color.foregroundColor, defaultForegroundColor, defaultBackgroundColor);

    let x =
      float_of_int(
        Constants.default.minimapCharacterWidth
        * startPosition
      );
    let height = float_of_int(Constants.default.minimapCharacterHeight);
    let width =
      float_of_int(tokenWidth * Constants.default.minimapCharacterWidth);

    Shapes.drawRect(
      ~transform,
      ~y=float_of_int(yOffset),
      ~x,
      ~color=foregroundColor,
      ~width,
      ~height,
      (),
    );
  };

  List.iter(f, tokens);
};

let renderLine = (theme, transform, yOffset, tokens, tokenColors, colorMap) => {
  tokensToElement(theme, transform, yOffset, tokens, tokenColors, colorMap);
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
          render={(transform, _) =>
            FlatList.render(
              ~scrollY,
              ~rowHeight,
              ~height,
              ~count,
              ~render=
                (item, offset) => {
                  let tokens = getTokensForLine(item);
                  let tokenColors = SyntaxHighlighting.getTokensForLine(state.syntaxHighlighting, state.activeBufferId, item);
                  renderLine(state.theme, transform, offset, tokens, tokenColors, state.syntaxHighlighting.colorMap);
                },
              (),
            )
          }
        />
      </View>,
    );
  });
