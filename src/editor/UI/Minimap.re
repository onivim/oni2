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
      top(Constants.default.minimapCharacterHeight * virtualLineNumber),
    ];

  let tokens = List.map(f, tokens);

  <View style=lineStyle> ...tokens </View>;
};

let viewLinesToElements = (bufferView: TokenizedBufferView.t, theme) => {
  let f = (b: BufferViewLine.t) => {
    tokensToElement(
      Index.toZeroBasedInt(b.virtualLineNumber),
      b.tokens,
      theme,
    );
  };

  Array.map(f, bufferView.viewLines) |> Array.to_list;
};

let component = React.component("Minimap");

let createElement =
    (
      ~state: State.t,
      ~tokenizedBufferView: TokenizedBufferView.t,
      ~children as _,
      (),
    ) =>
  component(hooks => {
    let style =
      Style.[position(`Absolute), top(0), bottom(0), left(0), right(0)];

    let elements = viewLinesToElements(tokenizedBufferView, state.theme);

    (hooks, <View style> ...elements </View>);
  });
