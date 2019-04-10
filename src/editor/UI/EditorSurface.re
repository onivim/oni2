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
open Oni_Model;

open Types;

let empty = React.listToElement([]);

/* Set up some styles */
let textHeaderStyle =
  Style.[fontFamily("FiraCode-Regular.ttf"), fontSize(14)];

/* Set up some styles */
let fontAwesomeStyle =
  Style.[fontFamily("FontAwesome5FreeRegular.otf"), fontSize(14)];

let fontAwesomeIcon = Zed_utf8.singleton(UChar.of_int(0xF556));

let renderLineNumber =
    (
      fontWidth: float,
      lineNumber: int,
      lineNumberWidth: float,
      theme: Theme.t,
      cursorLine: int,
      yOffset: float,
      transform,
    ) => {
  let isActiveLine = lineNumber == cursorLine;
  let lineNumberTextColor =
    isActiveLine
      ? theme.colors.editorActiveLineNumberForeground
      : theme.colors.editorLineNumberForeground;

  let yF = yOffset;

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
      ? 0.
      : lineNumberWidth
        /. 2.
        -. float_of_int(String.length(lineNumber))
        *. fontWidth
        /. 2.;

  Revery.Draw.Text.drawString(
    ~transform,
    ~x=lineNumberXOffset,
    ~y=yF,
    ~backgroundColor=theme.colors.editorLineNumberBackground,
    ~color=lineNumberTextColor,
    ~fontFamily="FiraCode-Regular.ttf",
    ~fontSize=14,
    lineNumber,
  );
};

let renderTokens =
    (
      fontWidth: float,
      _fontHeight: float,
      lineNumber: int,
      lineNumberWidth: float,
      theme: Theme.t,
      cursorLine: int,
      tokens,
      xOffset: float,
      yOffset: float,
      transform,
    ) => {
  let isActiveLine = lineNumber == cursorLine;
  /* let lineNumberTextColor = */
  /*   isActiveLine */
  /*     ? theme.colors.editorActiveLineNumberForeground */
  /*     : theme.colors.editorLineNumberForeground; */

  let yF = yOffset;
  let xF = xOffset;

  /* let lineNumber = */
  /*   string_of_int( */
  /*     LineNumber.getLineNumber( */
  /*       ~bufferLine=lineNumber + 1, */
  /*       ~cursorLine=cursorLine + 1, */
  /*       ~setting=Relative, */
  /*       (), */
  /*     ), */
  /*   ); */

  /*   let lineNumberXOffset = */
  /*     isActiveLine */
  /*       ? 0. : (lineNumberWidth /. 2) -. (float_of_int(String.length(lineNumber)) *. fontWidth /. 2.); */

  /*   Revery.Draw.Text.drawString( */
  /*     ~transform, */
  /*     ~x=lineNumberXOffset, */
  /*     ~y=yF, */
  /*     ~backgroundColor=theme.colors.editorLineNumberBackground, */
  /*     ~color=lineNumberTextColor, */
  /*     ~fontFamily="FiraCode-Regular.ttf", */
  /*     ~fontSize=14, */
  /*     lineNumber, */
  /*   ); */

  let textBackgroundColor =
    isActiveLine
      ? theme.colors.editorLineHighlightBackground : theme.colors.background;

  let f = (token: BufferViewTokenizer.t) => {
    Revery.Draw.Text.drawString(
      ~transform,
      ~x=
        lineNumberWidth
        +. fontWidth
        *. float_of_int(Index.toZeroBasedInt(token.startPosition))
        -. xF,
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
      BufferMap.getBuffer(state.activeBufferId, state.buffers);

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

    let iFontHeight = int_of_float(fontHeight +. 0.5);
    /* let iFontWidth = int_of_float(fontWidth +. 0.5); */
        let cursorLine = state.editor.cursorPosition.line;

    let (cursorOffset, cursorWidth) = if(Buffer.getNumberOfLines(buffer) > 0) {
        let cursorStr = Buffer.getLine(buffer, Index.toZeroBasedInt(state.editor.cursorPosition.line));

        let (cursorOffset, width) = BufferViewTokenizer.getCharacterPositionAndWidth(cursorStr, Index.toZeroBasedInt(state.editor.cursorPosition.character));
        let cursorWidth =
          switch (state.mode) {
          | Insert => 2
          | _ => width * int_of_float(fontWidth)
          };
        (cursorOffset, cursorWidth)
    } else {
        (0, 1);
    }

    let cursorStyle =
      Style.[
        position(`Absolute),
        top(
          int_of_float(
            fontHeight
            *. float_of_int(
                 Index.toZeroBasedInt(state.editor.cursorPosition.line),
               )
            -. state.editor.scrollY
            +. 0.5,
          ),
        ),
        left(
          int_of_float(
            lineNumberWidth
            +. fontWidth
            *. float_of_int(cursorOffset)
            -. state.editor.scrollX
            +. 0.5,
          ),
        ),
        height(iFontHeight),
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
      BufferViewTokenizer.tokenize(
        line,
        state.theme,
        tokenColors,
        state.syntaxHighlighting.colorMap,
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
        ~pixelWidth=float_of_int(state.editor.size.pixelWidth),
        ~pixelHeight=float_of_int(state.editor.size.pixelHeight),
        ~isMinimapShown=true,
        ~characterWidth=state.editorFont.measuredWidth,
        ~characterHeight=state.editorFont.measuredHeight,
        ~bufferLineCount=lineCount,
        (),
      );

    let bufferPixelWidth =
      layout.lineNumberWidthInPixels +. layout.bufferWidthInPixels;

    let bufferViewStyle =
      Style.[
        position(`Absolute),
        top(0),
        left(0),
        width(int_of_float(bufferPixelWidth)),
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
        left(int_of_float(bufferPixelWidth)),
        width(minimapPixelWidth),
        bottom(0),
      ];

    let verticalScrollBarStyle =
      Style.[
        position(`Absolute),
        top(0),
        left(
          int_of_float(bufferPixelWidth +. float_of_int(minimapPixelWidth)),
        ),
        width(Constants.default.scrollBarThickness),
        backgroundColor(theme.colors.scrollbarSliderBackground),
        bottom(0),
      ];

    let horizontalScrollBarStyle =
      Style.[
        position(`Absolute),
        bottom(0),
        left(int_of_float(layout.lineNumberWidthInPixels)),
        height(Constants.default.scrollBarThickness),
        width(int_of_float(layout.bufferWidthInPixels)),
      ];

    let scrollSurface = (wheelEvent: NodeEvents.mouseWheelEventParams) => {
      GlobalContext.current().editorScroll(
        ~deltaY=wheelEvent.deltaY *. (-50.),
        (),
      );
    };

    let scrollMinimap = (wheelEvent: NodeEvents.mouseWheelEventParams) => {
      GlobalContext.current().editorScroll(
        ~deltaY=wheelEvent.deltaY *. (-150.),
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

              /* Draw background for cursor line */
              Shapes.drawRect(
                ~transform,
                ~x=lineNumberWidth,
                ~y=
                  fontHeight
                  *. float_of_int(
                       Index.toZeroBasedInt(state.editor.cursorPosition.line),
                     )
                  -. state.editor.scrollY,
                ~height=fontHeight,
                ~width=
                  float_of_int(state.editor.size.pixelWidth)
                  -. lineNumberWidth,
                ~color=theme.colors.editorLineHighlightBackground,
                (),
              );

              /* Draw selection ranges */
              switch (activeBuffer) {
              | Some(b) =>
                let ranges = Selection.getRanges(state.editor.selection, b);
                Oni_Core.Types.Range.(
                  List.iter(
                    (r: Range.t) =>
                      Shapes.drawRect(
                        ~transform,
                        ~x=
                          lineNumberWidth
                          +. float_of_int(
                               Index.toZeroBasedInt(
                                 r.startPosition.character,
                               ),
                             )
                          *. fontWidth,
                        ~y=
                          fontHeight
                          *. float_of_int(
                               Index.toZeroBasedInt(r.startPosition.line),
                             )
                          -. state.editor.scrollY,
                        ~height=fontHeight,
                        ~width=
                          float_of_int(
                            Index.toZeroBasedInt(r.endPosition.character)
                            - Index.toZeroBasedInt(r.startPosition.character),
                          )
                          *. fontWidth,
                        ~color=theme.colors.editorSelectionBackground,
                        (),
                      ),
                    ranges,
                  )
                );
              | None => ()
              };

              FlatList.render(
                ~scrollY,
                ~rowHeight,
                ~height=float_of_int(height),
                ~count,
                ~render=
                  (item, offset) => {
                    let tokens = getTokensForLine(item);

                    let _ =
                      renderTokens(
                        fontWidth,
                        fontHeight,
                        item,
                        lineNumberWidth,
                        theme,
                        Index.toZeroBasedInt(cursorLine),
                        tokens,
                        state.editor.scrollX,
                        offset,
                        transform,
                      );
                    ();
                  },
                (),
              );

              /* Draw background for line numbers */
              Shapes.drawRect(
                ~transform,
                ~x=0.,
                ~y=0.,
                ~width=lineNumberWidth,
                ~height=float_of_int(height),
                ~color=theme.colors.editorLineNumberBackground,
                (),
              );

              FlatList.render(
                ~scrollY,
                ~rowHeight,
                ~height=float_of_int(height),
                ~count,
                ~render=
                  (item, offset) => {
                    let _ =
                      renderLineNumber(
                        fontWidth,
                        item,
                        lineNumberWidth,
                        theme,
                        Index.toZeroBasedInt(cursorLine),
                        offset,
                        transform,
                      );
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
              width={int_of_float(layout.bufferWidthInPixels)}
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
