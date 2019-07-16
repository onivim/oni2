/*
 * EditorSurface.re
 *
 * Component that handles rendering of the 'editor surface' -
 * the view of the buffer in the window.
 */

open Revery;
open Revery.Draw;
open Revery.UI;

open Oni_Core;
open Oni_Core.CamomileBundled.Camomile;
open Oni_Model;

open Types;

let empty = React.listToElement([]);

/* Set up some styles */
let textHeaderStyle =
  Style.[fontFamily("FiraCode-Regular.ttf"), fontSize(14)];

/* Set up some styles */
let fontAwesomeStyle =
  Style.[fontFamily("FontAwesome5FreeRegular.otf"), fontSize(14)];

let fontAwesomeIcon = ZedBundled.singleton(UChar.of_int(0xF556));

let renderLineNumber =
    (
      fontWidth: float,
      lineNumber: int,
      lineNumberWidth: float,
      theme: Theme.t,
      lineSetting,
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
        ~setting=lineSetting,
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
      whitespaceSetting: ConfigurationValues.editorRenderWhitespace,
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
      ~isActiveSplit: bool,
      ~editorGroupId: int,
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

    let showLineNumbers =
      Configuration.getValue(
        c => c.editorLineNumbers != LineNumber.Off,
        state.configuration,
      );
    let lineNumberWidth =
      showLineNumbers
        ? LineNumber.getLineNumberPixelWidth(
            ~lines=lineCount,
            ~fontPixelWidth=state.editorFont.measuredWidth,
            (),
          )
        : 0.0;

    let fontHeight = state.editorFont.measuredHeight;
    let fontWidth = state.editorFont.measuredWidth;

    let iFontHeight = int_of_float(fontHeight +. 0.5);
    let indentation =
      switch (Buffer.getIndentation(buffer)) {
      | Some(v) => v
      | None => IndentationSettings.default
      };

    let leftVisibleColumn = Editor.getLeftVisibleColumn(editor, metrics);
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

    let fullCursorWidth = cursorCharacterWidth * int_of_float(fontWidth);

    let cursorWidth =
      switch (state.mode, isActiveSplit) {
      | (Insert, true) => 2
      | _ => fullCursorWidth
      };

    let cursorOpacity = isActiveSplit ? 0.5 : 0.25;

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
        opacity(cursorOpacity),
        backgroundColor(Colors.white),
      ];

    let searchHighlights =
      Selectors.getSearchHighlights(state, editor.bufferId);

    let isMinimapShown =
      Configuration.getValue(
        c => c.editorMinimapEnabled,
        state.configuration,
      );

    let layout =
      EditorLayout.getLayout(
        ~showLineNumbers,
        ~maxMinimapCharacters=
          Configuration.getValue(
            c => c.editorMinimapMaxColumn,
            state.configuration,
          ),
        ~pixelWidth=float_of_int(metrics.pixelWidth),
        ~pixelHeight=float_of_int(metrics.pixelHeight),
        ~isMinimapShown,
        ~characterWidth=state.editorFont.measuredWidth,
        ~characterHeight=state.editorFont.measuredHeight,
        ~bufferLineCount=lineCount,
        (),
      );

    let matchingPairsEnabled =
      Selectors.getConfigurationValue(state, buffer, c =>
        c.editorMatchBrackets
      );

    let matchingPairs =
      !matchingPairsEnabled
        ? None : Selectors.getMatchingPairs(state, editor.bufferId);

    let getTokensForLine = (~selection=None, startIndex, endIndex, i) => {
      let line = Buffer.getLine(buffer, i);
      let tokenColors =
        SyntaxHighlighting.getTokensForLine(
          state.syntaxHighlighting,
          bufferId,
          i,
        );

      let searchHighlightRanges =
        switch (IntMap.find_opt(i, searchHighlights)) {
        | Some(v) => v
        | None => []
        };

      let isActiveLine = i == cursorLine;
      let defaultBackground =
        isActiveLine
          ? theme.colors.editorLineHighlightBackground
          : theme.colors.editorBackground;

      let matchingPairIndex =
        switch (matchingPairs) {
        | None => None
        | Some(v) =>
          if (Index.toInt0(v.startPos.line) == i) {
            Some(Index.toInt0(v.startPos.character));
          } else if (Index.toInt0(v.endPos.line) == i) {
            Some(Index.toInt0(v.endPos.character));
          } else {
            None;
          }
        };

      let colorizer =
        BufferLineColorizer.create(
          ZedBundled.length(line),
          state.theme,
          tokenColors,
          state.syntaxHighlighting.colorMap,
          selection,
          defaultBackground,
          theme.colors.editorSelectionBackground,
          matchingPairIndex,
          searchHighlightRanges,
        );

      BufferViewTokenizer.tokenize(
        ~startIndex,
        ~endIndex,
        line,
        IndentationSettings.default,
        colorizer,
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
      GlobalContext.current().notifyEditorSizeChanged(
        ~editorGroupId,
        ~width,
        ~height,
        (),
      );
    };

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

    let diagnostics =
      switch (activeBuffer) {
      | Some(b) => Diagnostics.getDiagnosticsMap(state.diagnostics, b)
      | None => IntMap.empty
      };
    let ranges = Selection.getRanges(editor.selection, buffer);
    let selectionRanges = Range.toHash(ranges);

    let minimapLayout =
      isMinimapShown
        ? <View style=minimapViewStyle onMouseWheel=scrollMinimap>
            <Minimap
              state
              editor
              width={layout.minimapWidthInPixels}
              height={metrics.pixelHeight}
              count=lineCount
              diagnostics
              metrics
              getTokensForLine={getTokensForLine(
                0,
                layout.bufferWidthInCharacters,
              )}
              selection=selectionRanges
            />
          </View>
        : React.empty;

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

              let renderRange = (~offset=0., ~color=Colors.black, r: Range.t) =>
                {let halfOffset = offset /. 2.0
                 let line = Index.toZeroBasedInt(r.startPosition.line)
                 let start = Index.toZeroBasedInt(r.startPosition.character)
                 let endC = Index.toZeroBasedInt(r.endPosition.character)

                 let text = Buffer.getLine(buffer, line)
                 let (startOffset, _) =
                   BufferViewTokenizer.getCharacterPositionAndWidth(
                     ~indentation,
                     ~viewOffset=leftVisibleColumn,
                     text,
                     start,
                   )
                 let (endOffset, _) =
                   BufferViewTokenizer.getCharacterPositionAndWidth(
                     ~indentation,
                     ~viewOffset=leftVisibleColumn,
                     text,
                     endC,
                   )

                 Shapes.drawRect(
                   ~transform,
                   ~x=
                     lineNumberWidth
                     +. float_of_int(startOffset)
                     *. fontWidth
                     -. halfOffset,
                   ~y=
                     fontHeight
                     *. float_of_int(
                          Index.toZeroBasedInt(r.startPosition.line),
                        )
                     -. editor.scrollY
                     -. halfOffset,
                   ~height=fontHeight +. offset,
                   ~width=
                     offset
                     +. max(float_of_int(endOffset - startOffset), 1.0)
                     *. fontWidth,
                   ~color,
                   (),
                 )};

              FlatList.render(
                ~scrollY,
                ~rowHeight,
                ~height=float_of_int(height),
                ~count,
                ~render=
                  (item, _offset) => {
                    let renderDiagnostics = (d: Diagnostics.Diagnostic.t) =>
                      renderRange(~color=Colors.red, d.range);

                    /* Draw error markers */
                    switch (IntMap.find_opt(item, diagnostics)) {
                    | None => ()
                    | Some(v) => List.iter(renderDiagnostics, v)
                    };

                    switch (Hashtbl.find_opt(selectionRanges, item)) {
                    | None => ()
                    | Some(v) =>
                      List.iter(
                        renderRange(
                          ~color=theme.colors.editorSelectionBackground,
                        ),
                        v,
                      )
                    };

                    /* Draw match highlights */
                    let matchColor = theme.colors.editorSelectionBackground;
                    switch (matchingPairs) {
                    | None => ()
                    | Some(v) =>
                      renderRange(
                        ~offset=0.0,
                        ~color=matchColor,
                        Range.createFromPositions(
                          ~startPosition=v.startPos,
                          ~endPosition=v.startPos,
                          (),
                        ),
                      );
                      renderRange(
                        ~offset=0.0,
                        ~color=matchColor,
                        Range.createFromPositions(
                          ~startPosition=v.endPos,
                          ~endPosition=v.endPos,
                          (),
                        ),
                      );
                    };

                    /* Draw search highlights */
                    switch (IntMap.find_opt(item, searchHighlights)) {
                    | None => ()
                    | Some(v) =>
                      List.iter(
                        r =>
                          renderRange(
                            ~offset=2.0,
                            ~color=theme.colors.editorFindMatchBackground,
                            r,
                          ),
                        v,
                      )
                    };
                  },
                (),
              );

              FlatList.render(
                ~scrollY,
                ~rowHeight,
                ~height=float_of_int(height),
                ~count,
                ~render=
                  (item, offset) => {
                    let selectionRange =
                      switch (Hashtbl.find_opt(selectionRanges, item)) {
                      | None => None
                      | Some(v) =>
                        switch (List.length(v)) {
                        | 0 => None
                        | _ => Some(List.hd(v))
                        }
                      };
                    let tokens =
                      getTokensForLine(
                        ~selection=selectionRange,
                        leftVisibleColumn,
                        leftVisibleColumn + layout.bufferWidthInCharacters,
                        item,
                      );

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
                        Configuration.getValue(
                          c => c.editorRenderWhitespace,
                          state.configuration,
                        ),
                      );
                    ();
                  },
                (),
              );

              /* Draw background for line numbers */
              if (showLineNumbers) {
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
                          Configuration.getValue(
                            c => c.editorLineNumbers,
                            state.configuration,
                          ),
                          cursorLine,
                          offset,
                          transform,
                        );
                      ();
                    },
                  (),
                );
              };

              let renderIndentGuides =
                Configuration.getValue(
                  c => c.editorRenderIndentGuides,
                  state.configuration,
                );
              let showActive =
                Configuration.getValue(
                  c => c.editorHighlightActiveIndentGuide,
                  state.configuration,
                );

              if (renderIndentGuides) {
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
                    ~showActive,
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
        minimapLayout
        <View style=verticalScrollBarStyle>
          <EditorVerticalScrollbar
            state
            editor
            metrics
            width={Constants.default.scrollBarThickness}
            height={metrics.pixelHeight}
            diagnostics
          />
        </View>
      </View>,
    );
  });
