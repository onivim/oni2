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
      lineNumber: int,
      virtualLineNumber: int,
      lineNumberWidth: int,
      tokens: list(Tokenizer.t),
      theme: Theme.t,
      cursorLine: int,
    ) => {
  let lineHeight = fontHeight;

  let isActiveLine = lineNumber == cursorLine;
  let lineNumberTextColor =
    isActiveLine
      ? theme.editorActiveLineNumberForeground
      : theme.editorLineNumberForeground;
  let lineNumberAlignment = isActiveLine ? `FlexStart : `Center;

  let f = (token: Tokenizer.t) => {
    let style =
      Style.[
        position(`Absolute),
        top(0),
        left(fontWidth * Index.toZeroBasedInt(token.startPosition)),
        fontFamily("FiraCode-Regular.ttf"),
        fontSize(14),
        height(fontHeight),
      ];

    <Text style text={token.text} />;
  };

  let lineStyle =
    Style.[position(`Absolute), top(fontHeight * virtualLineNumber)];

  let lineNumberStyle =
    Style.[
      position(`Absolute),
      top(0),
      height(lineHeight),
      left(0),
      width(lineNumberWidth),
      backgroundColor(theme.editorLineNumberBackground),
      justifyContent(`Center),
      alignItems(lineNumberAlignment),
    ];

  let lineContentsStyle =
    Style.[
      position(`Absolute),
      top(0),
      left(lineNumberWidth),
      right(0),
      height(lineHeight),
    ];

  let lineNumberTextStyle =
    Style.[
      fontFamily("FiraCode-Regular.ttf"),
      fontSize(14),
      height(fontHeight),
      color(lineNumberTextColor),
    ];

  let tokens = List.map(f, tokens);

  <View style=lineStyle>
    <View style=lineNumberStyle>
      <Text
        style=lineNumberTextStyle
        text={string_of_int(
          LineNumber.getLineNumber(
            ~bufferLine=lineNumber + 1,
            ~cursorLine=cursorLine + 1,
            ~setting=Relative,
            (),
          ),
        )}
      />
    </View>
    <View style=lineContentsStyle> ...tokens </View>
  </View>;
};

let viewLinesToElements =
    (
      fontWidth: int,
      fontHeight: int,
      lineNumberWidth: int,
      bufferView: TokenizedBufferView.t,
      theme,
      cursorLine,
    ) => {
  let f = (b: BufferViewLine.t) => {
    tokensToElement(
      fontWidth,
      fontHeight,
      Index.toZeroBasedInt(b.lineNumber),
      Index.toZeroBasedInt(b.virtualLineNumber),
      lineNumberWidth,
      b.tokens,
      theme,
      Index.toZeroBasedInt(cursorLine),
    );
  };

  Array.map(f, bufferView.viewLines) |> Array.to_list;
};

let component = React.component("EditorSurface");

let createElement = (~state: State.t, ~children as _, ()) =>
  component((_slots: React.Hooks.empty) => {
    let theme = state.theme;

    let lineNumberWidth =
      LineNumber.getLineNumberPixelWidth(
        ~lines=Array.length(state.buffer.lines),
        ~fontPixelWidth=state.editorFont.measuredWidth,
        (),
      );

    let bufferView =
      state.buffer
      |> TokenizedBuffer.ofBuffer
      |> TokenizedBufferView.ofTokenizedBuffer;

    let textElements =
      viewLinesToElements(
        state.editorFont.measuredWidth,
        state.editorFont.measuredHeight,
        lineNumberWidth,
        bufferView,
        state.theme,
        state.cursorPosition.line,
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
          lineNumberWidth
          + fontWidth
          * Index.toZeroBasedInt(state.cursorPosition.character),
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

    <View style> ...elements </View>;
  });
