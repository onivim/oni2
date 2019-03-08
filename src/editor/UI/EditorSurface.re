/*
 * EditorSurface.re
 *
 * Component that handles rendering of the 'editor surface' -
 * the view of the buffer in the window.
 */

open Revery;
open Revery.Draw;
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
      _fontHeight: int,
      lineNumber: int,
      lineNumberWidth: int,
      theme: Theme.t,
      cursorLine: int,
      tokens,
      yOffset: int,
      transform,
    ) => {
  let isActiveLine = lineNumber == cursorLine;
  let lineNumberTextColor =
    isActiveLine
      ? theme.colors.editorActiveLineNumberForeground
      : theme.colors.editorLineNumberForeground;

  let yF = float_of_int(yOffset);

  let lineNumber =
    string_of_int(
      LineNumber.getLineNumber(
        ~bufferLine=lineNumber + 1,
        ~cursorLine=cursorLine + 1,
        ~setting=Relative,
        (),
      ),
    );

  let lineNumberXOffset =
    isActiveLine
      ? 0 : lineNumberWidth / 2 - String.length(lineNumber) * fontWidth / 2;

  Revery.Draw.Text.drawString(
    ~transform,
    ~x=float_of_int(lineNumberXOffset),
    ~y=yF,
    ~backgroundColor=theme.colors.editorLineNumberBackground,
    ~color=lineNumberTextColor,
    ~fontFamily="FiraCode-Regular.ttf",
    ~fontSize=14,
    lineNumber,
  );

  let textBackgroundColor =
    isActiveLine
      ? theme.colors.editorLineHighlightBackground : theme.colors.background;

  let f = (token: Tokenizer.t) => {
    Revery.Draw.Text.drawString(
      ~transform,
      ~x=
        float_of_int(
          lineNumberWidth
          + fontWidth
          * Index.toZeroBasedInt(token.startPosition),
        ),
      ~y=yF,
      ~backgroundColor=textBackgroundColor,
      ~color=token.color,
      ~fontFamily="FiraCode-Regular.ttf",
      ~fontSize=14,
      token.text,
    );
  };

  List.iter(f, tokens);
};

let component = React.component("EditorSurface");

let createElement = (~state: State.t, ~children as _, ()) =>
  component(hooks => {
    let theme = state.theme;

    let activeBuffer =
      Oni_Core.BufferMap.getBuffer(state.activeBufferId, state.buffers);

    let buffer =
      switch (activeBuffer) {
      | Some(buffer) => buffer
      | None => Buffer.empty
      };

    let lineCount = Buffer.getNumberOfLines(buffer);

    let lineNumberWidth =
      LineNumber.getLineNumberPixelWidth(
        ~lines=lineCount,
        ~fontPixelWidth=state.editorFont.measuredWidth,
        (),
      );

    let fontHeight = state.editorFont.measuredHeight;
    let fontWidth = state.editorFont.measuredWidth;

    let cursorLine = state.editor.cursorPosition.line;
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
          * Index.toZeroBasedInt(state.editor.cursorPosition.line)
          - state.editor.scrollY,
        ),
        left(
          lineNumberWidth
          + fontWidth
          * Index.toZeroBasedInt(state.editor.cursorPosition.character),
        ),
        height(fontHeight),
        width(cursorWidth),
        opacity(0.8),
        backgroundColor(Colors.white),
      ];

    let getTokensForLine = i => {
      let line = Buffer.getLine(buffer, i);
      let tokenColors =
        SyntaxHighlighting.getTokensForLine(
          state.syntaxHighlighting,
          state.activeBufferId,
          i,
        );
      Tokenizer.tokenize(
        line,
        state.theme,
        tokenColors,
        state.syntaxHighlighting.colorMap,
      );
    };

    let render = (i, offset, transform) => {
      let tokens = getTokensForLine(i);

      tokensToElement(
        fontWidth,
        fontHeight,
        i,
        lineNumberWidth,
        theme,
        Index.toZeroBasedInt(cursorLine),
        tokens,
        offset,
        transform,
      );
    };

    let style =
      Style.[
        backgroundColor(theme.colors.background),
        color(theme.colors.foreground),
        flexGrow(1),
      ];

    let onDimensionsChanged =
        ({width, height}: NodeEvents.DimensionsChangedEventParams.t) => {
      GlobalContext.current().notifySizeChanged(~width, ~height, ());
    };

    let layout =
      EditorLayout.getLayout(
        ~pixelWidth=state.editor.size.pixelWidth,
        ~pixelHeight=state.editor.size.pixelHeight,
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
    let minimapViewStyle =
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
        backgroundColor(theme.colors.scrollbarSliderBackground),
        bottom(0),
      ];

    let horizontalScrollBarStyle =
      Style.[
        position(`Absolute),
        bottom(0),
        left(layout.lineNumberWidthInPixels),
        height(Constants.default.scrollBarThickness),
        width(layout.bufferWidthInPixels),
      ];

    let scrollSurface = (wheelEvent: NodeEvents.mouseWheelEventParams) => {
      GlobalContext.current().editorScroll(
        ~deltaY=int_of_float(wheelEvent.deltaY) * (-50),
        (),
      );
    };

    let scrollMinimap = (wheelEvent: NodeEvents.mouseWheelEventParams) => {
      GlobalContext.current().editorScroll(
        ~deltaY=int_of_float(wheelEvent.deltaY) * (-150),
        (),
      );
    };

    (
      hooks,
      <View style onDimensionsChanged>
        <View style=bufferViewStyle onMouseWheel=scrollSurface>
          <OpenGL
            style=bufferViewStyle
            render={(transform, _ctx) => {
              let count = lineCount;
              let height = state.editor.size.pixelHeight;
              let rowHeight = state.editorFont.measuredHeight;
              let scrollY = state.editor.scrollY;

              /* Draw background for line numbers */
              Shapes.drawRect(
                ~transform,
                ~x=0.,
                ~y=0.,
                ~width=float_of_int(lineNumberWidth),
                ~height=float_of_int(height),
                ~color=theme.colors.editorLineNumberBackground,
                (),
              );

              /* Draw background for cursor line */
              Shapes.drawRect(
                ~transform,
                ~x=float_of_int(lineNumberWidth),
                ~y=
                  float_of_int(
                    fontHeight
                    * Index.toZeroBasedInt(state.editor.cursorPosition.line)
                    - state.editor.scrollY,
                  ),
                ~height=float_of_int(fontHeight),
                ~width=
                  float_of_int(
                    state.editor.size.pixelWidth - lineNumberWidth,
                  ),
                ~color=theme.colors.editorLineHighlightBackground,
                (),
              );

              FlatList.render(
                ~scrollY,
                ~rowHeight,
                ~height,
                ~count,
                ~render=
                  (item, offset) => {
                    let _ = render(item, offset, transform);
                    ();
                  },
                (),
              );
            }}
          />
          <View style=cursorStyle />
          <View style=horizontalScrollBarStyle>
            <EditorHorizontalScrollbar
              state
              width={layout.bufferWidthInPixels}
            />
          </View>
        </View>
        <View style=minimapViewStyle onMouseWheel=scrollMinimap>
          <Minimap
            state
            width={layout.minimapWidthInPixels}
            height={state.editor.size.pixelHeight}
            count=lineCount
            getTokensForLine
          />
        </View>
        <View style=verticalScrollBarStyle>
          <EditorVerticalScrollbar
            state
            height={state.editor.size.pixelHeight}
            width={Constants.default.scrollBarThickness}
          />
        </View>
      </View>,
    );
  });
