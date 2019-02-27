/*
 * EditorSurface.re
 *
 * Component that handles rendering of the 'editor surface' -
 * the view of the buffer in the window.
 */

open Revery;
open Revery.UI;

open CamomileLibraryDefault.Camomile;

open Oni_Core;

open Types;

let empty = React.listToElement([]);

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
      lineNumberWidth: int,
      theme: Theme.t,
      cursorLine: int,
      tokens,
    ) => {
  let fontLineHeight = fontHeight;

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
        lineHeight(1.0),
        color(Revery.Colors.white),
        textWrap(Revery.TextWrapping.NoWrap),
      ];

    <Text style text={token.text} />;
  };

  let lineStyle = Style.[position(`Absolute), top(0), left(0), right(0)];

  let lineNumberStyle =
    Style.[
      position(`Absolute),
      top(0),
      height(fontLineHeight),
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
      height(fontLineHeight),
    ];

  let lineNumberTextStyle =
    Style.[
      fontFamily("FiraCode-Regular.ttf"),
      fontSize(14),
      height(fontHeight),
      color(lineNumberTextColor),
      lineHeight(1.0),
      textWrap(Revery.TextWrapping.NoWrap),
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

let component = React.component("EditorSurface");

let createElement = (~state: State.t, ~children as _, ()) =>
  component(hooks => {
    let theme = state.theme;

    let activeBuffer =
      Oni_Core.BufferMap.getBuffer(state.activeBufferId, state.buffers);
    let lineCount = Array.length(activeBuffer.lines);

    let lineNumberWidth =
      LineNumber.getLineNumberPixelWidth(
        ~lines=lineCount,
        ~fontPixelWidth=state.editorFont.measuredWidth,
        (),
      );

    let fontHeight = state.editorFont.measuredHeight;
    let fontWidth = state.editorFont.measuredWidth;

    let cursorLine = state.editorView.cursorPosition.line;
    let cursorWidth =
      switch (state.mode) {
      | Insert => 2
      | _ => fontWidth
      };

    let cursorStyle =
      Style.[
        position(`Absolute),
        top(
          fontHeight
          * Index.toZeroBasedInt(state.editorView.cursorPosition.line)
          - state.editorView.scrollY,
        ),
        left(
          lineNumberWidth
          + fontWidth
          * Index.toZeroBasedInt(state.editorView.cursorPosition.character),
        ),
        height(fontHeight),
        width(cursorWidth),
        opacity(0.8),
        backgroundColor(Colors.white),
      ];

    let getTokensForLine = i => {
      let line = activeBuffer.lines[i];
      Tokenizer.tokenize(line);
    };

    let render = i => {
      let tokens = getTokensForLine(i);

      tokensToElement(
        fontWidth,
        fontHeight,
        i,
        lineNumberWidth,
        theme,
        Index.toZeroBasedInt(cursorLine),
        tokens,
      );
    };

    let style =
      Style.[
        backgroundColor(theme.background),
        color(theme.foreground),
        flexGrow(1),
      ];

    let onDimensionsChanged =
        ({width, height}: NodeEvents.DimensionsChangedEventParams.t) =>
      GlobalContext.current().notifySizeChanged(~width, ~height, ());

    let layout =
      EditorLayout.getLayout(
        ~pixelWidth=state.editorView.size.pixelWidth,
        ~pixelHeight=state.editorView.size.pixelHeight,
        ~isMinimapShown=true,
        ~characterWidth=state.editorFont.measuredWidth,
        ~characterHeight=state.editorFont.measuredHeight,
        ~bufferLineCount=lineCount,
        (),
      );

    let bufferPixelWidth =
      layout.lineNumberWidthInPixels + layout.bufferWidthInPixels;

    let bufferViewStyle =
      Style.[
        position(`Absolute),
        top(0),
        left(0),
        width(bufferPixelWidth),
        bottom(0),
        overflow(`Hidden),
      ];

    let minimapPixelWidth =
      layout.minimapWidthInPixels + Constants.default.minimapPadding * 2;
    let _minimapViewStyle =
      Style.[
        position(`Absolute),
        overflow(`Hidden),
        top(0),
        left(bufferPixelWidth),
        width(minimapPixelWidth),
        bottom(0),
      ];

    let verticalScrollBarStyle =
      Style.[
        position(`Absolute),
        top(0),
        left(bufferPixelWidth + minimapPixelWidth),
        width(Constants.default.scrollBarThickness),
        backgroundColor(theme.scrollbarSliderBackground),
        bottom(0),
      ];

    (
      hooks,
      <View style onDimensionsChanged>
        <View style=bufferViewStyle>
          <FlatList
            render
            count=lineCount
            width=bufferPixelWidth
            height={state.editorView.size.pixelHeight}
            rowHeight={state.editorFont.measuredHeight}
            scrollY={state.editorView.scrollY}
          />
          <View style=cursorStyle />
        </View>
        <View style=verticalScrollBarStyle>
          <EditorVerticalScrollbar
            state
            height={state.editorView.size.pixelHeight}
            width={Constants.default.scrollBarThickness}
          />
        </View>
      </View>,
    );
  });
