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

let renderSpaces =
    (
      ~fontWidth: float,
      ~fontHeight: float,
      ~x: float,
      ~y: float,
      ~transform,
      ~count: int,
      ~theme: Theme.t,
      (),
    ) => {
  let i = ref(0);

  let size = 2.;
  let xOffset = fontWidth /. 2. -. 1.;
  let yOffset = fontHeight /. 2. -. 1.;

  while (i^ < count) {
    let iF = float_of_int(i^);
    let xPos = x +. fontWidth *. iF;

    Shapes.drawRect(
      ~transform,
      ~x=xPos +. xOffset,
      ~y=y +. yOffset,
      ~width=size,
      ~height=size,
      ~color=theme.colors.editorWhitespaceForeground,
      (),
    );

    incr(i);
  };
};

let renderTokens =
    (
      fontWidth: float,
      fontHeight: float,
      lineNumberWidth: float,
      theme: Theme.t,
      tokens,
      xOffset: float,
      yOffset: float,
      transform,
      whitespaceSetting: Configuration.editorRenderWhitespace,
    ) => {
  let yF = yOffset;
  let xF = xOffset;

  let f = (token: BufferViewTokenizer.t) => {
    let x =
      lineNumberWidth
      +. fontWidth
      *. float_of_int(Index.toZeroBasedInt(token.startPosition))
      -. xF;
    let y = yF;

    let backgroundColor = token.backgroundColor;

    switch (token.tokenType) {
    | Text =>
      Revery.Draw.Text.drawString(
        ~transform,
        ~x,
        ~y,
        ~backgroundColor,
        ~color=token.color,
        ~fontFamily="FiraCode-Regular.ttf",
        ~fontSize=14,
        token.text,
      )
    | Tab =>
      Revery.Draw.Text.drawString(
        ~transform,
        ~x=x +. fontWidth /. 4.,
        ~y=y +. fontHeight /. 4.,
        ~backgroundColor,
        ~color=theme.colors.editorWhitespaceForeground,
        ~fontFamily="FontAwesome5FreeSolid.otf",
        ~fontSize=10,
        FontIcon.codeToIcon(0xf30b),
      )
    | Whitespace =>
      renderSpaces(
        ~fontWidth,
        ~fontHeight,
        ~x,
        ~y,
        ~transform,
        ~count=String.length(token.text),
        ~theme,
        (),
      )
    };
  };

  tokens |> WhitespaceTokenFilter.filter(whitespaceSetting) |> List.iter(f);
};

let component = React.component("EditorSurface");

let createElement =
    (
      ~state: State.t,
      ~metrics: EditorMetrics.t,
      ~editor: Editor.t,
      ~children as _,
      (),
    ) =>
  component(hooks => {
    let theme = state.theme;

    let activeBuffer = Selectors.getBufferForEditor(state, editor);

    let buffer =
      switch (activeBuffer) {
      | Some(buffer) => buffer
      | None => Buffer.empty
      };

    let bufferId = Buffer.getId(buffer);
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
    let indentation = IndentationSettings.default;

    let topVisibleLine = Editor.getTopVisibleLine(editor, metrics);
    let bottomVisibleLine = Editor.getBottomVisibleLine(editor, metrics);

    let cursorLine = Index.toZeroBasedInt(editor.cursorPosition.line);

    let (cursorOffset, cursorCharacterWidth) =
      if (lineCount > 0 && cursorLine < lineCount) {
        let cursorStr = Buffer.getLine(buffer, cursorLine);

        let (cursorOffset, width) =
          BufferViewTokenizer.getCharacterPositionAndWidth(
            ~indentation,
            cursorStr,
            Index.toZeroBasedInt(editor.cursorPosition.character),
          );
        (cursorOffset, width);
      } else {
        (0, 1);
      };

    let bufferPositionToPixel = (line, char) => {
      let x =
        float_of_int(char) *. fontWidth -. editor.scrollX +. lineNumberWidth;
      let y = float_of_int(line) *. fontHeight -. editor.scrollY;
      (x, y);
    };

    let cursorWidth =
      switch (state.mode) {
      | Insert => 2
      | _ => cursorCharacterWidth * int_of_float(fontWidth)
      };

    let cursorStyle =
      Style.[
        position(`Absolute),
        top(
          int_of_float(
            fontHeight
            *. float_of_int(
                 Index.toZeroBasedInt(editor.cursorPosition.line),
               )
            -. editor.scrollY
            +. 0.5,
          ),
        ),
        left(
          int_of_float(
            lineNumberWidth
            +. fontWidth
            *. float_of_int(cursorOffset)
            -. editor.scrollX
            +. 0.5,
          ),
        ),
        height(iFontHeight),
        width(cursorWidth),
        opacity(0.5),
        backgroundColor(Colors.white),
      ];

    let getTokensForLine = (~selection=None, i) => {
      let line = Buffer.getLine(buffer, i);
      let tokenColors =
        SyntaxHighlighting.getTokensForLine(
          state.syntaxHighlighting,
          bufferId,
          i,
        );

      let isActiveLine = i == cursorLine;
      let defaultBackground =
        isActiveLine
          ? theme.colors.editorLineHighlightBackground
          : theme.colors.editorBackground;

      BufferViewTokenizer.tokenize(
        line,
        state.theme,
        tokenColors,
        state.syntaxHighlighting.colorMap,
        IndentationSettings.default,
        selection,
        defaultBackground,
        theme.colors.editorSelectionBackground,
      );
    };

    let style =
      Style.[
        backgroundColor(theme.colors.editorBackground),
        color(theme.colors.editorForeground),
        flexGrow(1),
      ];

    let onDimensionsChanged =
        ({width, height}: NodeEvents.DimensionsChangedEventParams.t) => {
      GlobalContext.current().notifySizeChanged(~width, ~height, ());
    };

    let layout =
      EditorLayout.getLayout(
        ~maxMinimapCharacters=state.configuration.editorMinimapMaxColumn,
        ~pixelWidth=float_of_int(metrics.pixelWidth),
        ~pixelHeight=float_of_int(metrics.pixelHeight),
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
              let height = metrics.pixelHeight;
              let rowHeight = metrics.lineHeight;
              let scrollY = editor.scrollY;

              /* Draw background for cursor line */
              Shapes.drawRect(
                ~transform,
                ~x=lineNumberWidth,
                ~y=
                  fontHeight
                  *. float_of_int(
                       Index.toZeroBasedInt(editor.cursorPosition.line),
                     )
                  -. editor.scrollY,
                ~height=fontHeight,
                ~width=float_of_int(metrics.pixelWidth) -. lineNumberWidth,
                ~color=theme.colors.editorLineHighlightBackground,
                (),
              );

              let selectionRanges: Hashtbl.t(int, Range.t) =
                Hashtbl.create(100);

              /* Draw selection ranges */
              switch (activeBuffer) {
              | Some(b) =>
                let ranges = Selection.getRanges(editor.selection, b);
                Oni_Core.Types.Range.(
                  List.iter(
                    (r: Range.t) => {
                      let line = Index.toZeroBasedInt(r.startPosition.line);
                      let start =
                        Index.toZeroBasedInt(r.startPosition.character);
                      let endC =
                        Index.toZeroBasedInt(r.endPosition.character);

                      let text = Buffer.getLine(b, line);
                      let (startOffset, _) =
                        BufferViewTokenizer.getCharacterPositionAndWidth(
                          ~indentation,
                          text,
                          start,
                        );
                      let (endOffset, _) =
                        BufferViewTokenizer.getCharacterPositionAndWidth(
                          ~indentation,
                          text,
                          endC,
                        );

                      Hashtbl.add(selectionRanges, line, r);
                      Shapes.drawRect(
                        ~transform,
                        ~x=
                          lineNumberWidth
                          +. float_of_int(startOffset)
                          *. fontWidth,
                        ~y=
                          fontHeight
                          *. float_of_int(
                               Index.toZeroBasedInt(r.startPosition.line),
                             )
                          -. editor.scrollY,
                        ~height=fontHeight,
                        ~width=
                          max(float_of_int(endOffset - startOffset), 1.0)
                          *. fontWidth,
                        ~color=theme.colors.editorSelectionBackground,
                        (),
                      );
                    },
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
                    let selectionRange =
                      Hashtbl.find_opt(selectionRanges, item);
                    let tokens =
                      getTokensForLine(~selection=selectionRange, item);

                    let _ =
                      renderTokens(
                        fontWidth,
                        fontHeight,
                        lineNumberWidth,
                        theme,
                        tokens,
                        editor.scrollX,
                        offset,
                        transform,
                        state.configuration.editorRenderWhitespace,
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
                        cursorLine,
                        offset,
                        transform,
                      );
                    ();
                  },
                (),
              );

              if (state.configuration.editorRenderIndentGuides) {
                switch (activeBuffer) {
                | None => ()
                | Some(buffer) =>
                  IndentLineRenderer.render(
                    ~transform,
                    ~buffer,
                    ~startLine=topVisibleLine - 1,
                    ~endLine=bottomVisibleLine + 1,
                    ~lineHeight=fontHeight,
                    ~fontWidth,
                    ~cursorLine=
                      Index.toZeroBasedInt(editor.cursorPosition.line),
                    ~theme=state.theme,
                    ~indentationSettings=indentation,
                    ~bufferPositionToPixel,
                    ~showActive=
                      state.configuration.editorHighlightActiveIndentGuide,
                    (),
                  )
                };
              };
            }}
          />
          <View style=cursorStyle />
          <View style=horizontalScrollBarStyle>
            <EditorHorizontalScrollbar
              editor
              state
              metrics
              width={int_of_float(layout.bufferWidthInPixels)}
            />
          </View>
        </View>
        <View style=minimapViewStyle onMouseWheel=scrollMinimap>
          <Minimap
            state
            editor
            width={layout.minimapWidthInPixels}
            height={metrics.pixelHeight}
            count=lineCount
            metrics
            getTokensForLine
          />
        </View>
        <View style=verticalScrollBarStyle>
          <EditorVerticalScrollbar
            state
            editor
            metrics
            width={Constants.default.scrollBarThickness}
            height={metrics.pixelHeight}
          />
        </View>
      </View>,
    );
  });
