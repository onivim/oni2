/*
 * Minimap.re
 *
 * Component that handles Minimap rendering
 */

open Revery.UI;

open Oni_Core;
open Oni_Core.TokenizedBufferView;

open Types;

let tokensToElement =
    (virtualLineNumber: int, tokens: list(Tokenizer.t), theme: Theme.t) => {
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

  let lineStyle =
    Style.[
      position(`Absolute),
      top(
        (
          Constants.default.minimapCharacterHeight
          + Constants.default.minimapLineSpacing
        )
        * virtualLineNumber,
      ),
    ];

  let tokens = List.map(f, tokens);

  <View style=lineStyle> ...tokens </View>;
};

let renderLine = (theme, b: BufferViewLine.t) => {
  tokensToElement(
    Index.toZeroBasedInt(b.virtualLineNumber),
    b.tokens,
    theme,
  );
};

let component = React.component("Minimap");

let createElement =
    (
      ~state: State.t,
      ~width: int,
      ~height: int,
      ~tokenizedBufferView: TokenizedBufferView.t,
      ~children as _,
      (),
    ) =>
  component(hooks => {
    let style =
      Style.[position(`Absolute), top(0), bottom(0), left(0), right(0)];

    let rowHeight =
      Constants.default.minimapCharacterHeight
      + Constants.default.minimapLineSpacing;
    let render = renderLine(state.theme);

    (
      hooks,
      <View style>
        <FlatList
          width
          height
          rowHeight
          render
          data={tokenizedBufferView.viewLines}
        />
      </View>,
    );
  });
