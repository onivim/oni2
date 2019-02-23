/*
 * Minimap.re
 *
 * Component that handles Minimap rendering
 */

open Revery.UI;

open Oni_Core;

open Types;

let lineStyle = Style.[position(`Absolute), top(0)];

let tokensToElement = (tokens: list(Tokenizer.t), theme: Theme.t) => {
  let f = (token: Tokenizer.t) => {
    let tokenWidth =
      Index.toZeroBasedInt(token.endPosition)
      - Index.toZeroBasedInt(token.startPosition);
    let style =
      Style.[
        position(`Absolute),
        top(0),
        left(
          Constants.default.minimapCharacterWidth
          * Index.toZeroBasedInt(token.startPosition),
        ),
        height(Constants.default.minimapCharacterHeight),
        width(tokenWidth * Constants.default.minimapCharacterWidth),
        backgroundColor(theme.editorForeground),
      ];

    <View style />;
  };

  let tokens = List.map(f, tokens);

  <View style=lineStyle> ...tokens </View>;
};

let renderLine = (theme, tokens) => {
  tokensToElement(tokens, theme);
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
    let render = i => {
      let tokens = getTokensForLine(i);
      renderLine(state.theme, tokens);
    };

    (
      hooks,
      <View style=absoluteStyle>
        <FlatList
          width
          height
          rowHeight
          render
          count
          scrollY={state.editorView.minimapScrollY}
        />
      </View>,
    );
  });
