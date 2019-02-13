/*
 * EditorSurface.re
 *
 * Component that handles rendering of the 'editor surface' -
 * the view of the buffer in the window.
 */

open Revery.Core;
open Revery.UI;

open CamomileLibraryDefault.Camomile;

open Oni_Core;
open Oni_Core.TokenizedBufferView;

open Types;

/* Set up some styles */
let textHeaderStyle =
  Style.[fontFamily("FiraCode-Regular.ttf"), fontSize(14)];

/* Set up some styles */
let fontAwesomeStyle =
  Style.[fontFamily("FontAwesome5FreeRegular.otf"), fontSize(14)];

let fontAwesomeIcon = Zed_utf8.singleton(UChar.of_int(0xF556));

let tokensToElement =
    (
      fontWidth: int,
      fontHeight: int,
      virtualLineNumber: int,
      tokens: list(Tokenizer.t),
    ) => {
  let f = (token: Tokenizer.t) => {
    let style =
      Style.[
        position(`Absolute),
        top(fontHeight * virtualLineNumber),
        left(fontWidth * Index.toZeroBasedInt(token.startPosition)),
        fontFamily("FiraCode-Regular.ttf"),
        fontSize(14),
        height(fontHeight),
      ];

    <Text style text={token.text} />;
  };

  List.map(f, tokens);
};

let viewLinesToElements =
    (fontWidth: int, fontHeight: int, bufferView: TokenizedBufferView.t) => {
  let f = (b: BufferViewLine.t) => {
    tokensToElement(
      fontWidth,
      fontHeight,
      Index.toZeroBasedInt(b.virtualLineNumber),
      b.tokens,
    );
  };

  Array.map(f, bufferView.viewLines) |> Array.to_list |> List.flatten;
};

let component = React.component("EditorSurface");

let createElement = (~state: State.t, ~children as _, ()) =>
  component((hooks) => {
    let theme = Theme.get();

    let bufferView =
      state.buffer
      |> TokenizedBuffer.ofBuffer
      |> TokenizedBufferView.ofTokenizedBuffer;

    let textElements =
      viewLinesToElements(
        state.editorFont.measuredWidth,
        state.editorFont.measuredHeight,
        bufferView,
      );

    let fontHeight = state.editorFont.measuredHeight;
    let fontWidth = state.editorFont.measuredWidth;

    let cursorWidth =
      switch (state.mode) {
      | Insert => 2
      | _ => fontWidth
      };

    let cursorStyle =
      Style.[
        position(`Absolute),
        top(fontHeight * Index.toZeroBasedInt(state.cursorPosition.line)),
        left(
          fontWidth * Index.toZeroBasedInt(state.cursorPosition.character),
        ),
        height(fontHeight),
        width(cursorWidth),
        opacity(0.8),
        backgroundColor(Colors.white),
      ];

    let elements = [<View style=cursorStyle />, ...textElements];

    let style =
      Style.[
        backgroundColor(theme.background),
        color(theme.foreground),
        flexGrow(1),
      ];

    (hooks, <View style> ...elements </View>);
  });
