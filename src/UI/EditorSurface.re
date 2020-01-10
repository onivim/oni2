/*
 * EditorSurface.re
 *
 * Component that handles rendering of the 'editor surface' -
 * the view of the buffer in the window.
 */

open EditorCoreTypes;
open Revery;
open Revery.Draw;
open Revery.UI;

open Oni_Core;
open Oni_Core.CamomileBundled.Camomile;
open Oni_Model;

/* Set up some styles */
let textHeaderStyle =
  Style.[fontFamily("FiraCode-Regular.ttf"), fontSize(14)];

/* Set up some styles */
let fontAwesomeStyle =
  Style.[fontFamily("FontAwesome5FreeRegular.otf"), fontSize(14)];

let fontAwesomeIcon = ZedBundled.singleton(UChar.of_int(0xF556));

module Styles = {
  let bufferViewCommon = bufferPixelWidth =>
    Style.[
      position(`Absolute),
      top(0),
      left(0),
      width(int_of_float(bufferPixelWidth)),
      bottom(0),
    ];

  let bufferViewOverlay = bufferPixelWidth =>
    Style.[pointerEvents(`Ignore), ...bufferViewCommon(bufferPixelWidth)];

  let bufferViewClipped = bufferPixelWidth =>
    Style.[overflow(`Hidden), ...bufferViewCommon(bufferPixelWidth)];
};

let renderLineNumber =
    (
      fontFamily: string,
      fontSize: int,
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
      ? theme.editorActiveLineNumberForeground
      : theme.editorLineNumberForeground;

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
    ~window=Revery.UI.getActiveWindow(),
    ~transform,
    ~x=lineNumberXOffset,
    ~y=yF,
    ~backgroundColor=theme.editorLineNumberBackground,
    ~color=lineNumberTextColor,
    ~fontFamily,
    ~fontSize,
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
      ~color=theme.editorWhitespaceForeground,
      (),
    );

    incr(i);
  };
};

let renderTokens =
    (
      fontFamily: string,
      fontSize: int,
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
      *. float_of_int(Index.toZeroBased(token.startPosition))
      -. xF;
    let y = yF;

    let backgroundColor = token.backgroundColor;

    switch (token.tokenType) {
    | Text =>
      Revery.Draw.Text.drawString(
        ~window=Revery.UI.getActiveWindow(),
        ~transform,
        ~x,
        ~y,
        ~backgroundColor,
        ~color=token.color,
        ~fontFamily,
        ~fontSize,
        token.text,
      )
    | Tab =>
      Revery.Draw.Text.drawString(
        ~window=Revery.UI.getActiveWindow(),
        ~transform,
        ~x=x +. fontWidth /. 4.,
        ~y=y +. fontHeight /. 4.,
        ~backgroundColor,
        ~color=theme.editorWhitespaceForeground,
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

let%component make =
              (
                ~state: State.t,
                ~isActiveSplit: bool,
                ~editorGroup: EditorGroup.t,
                ~metrics: EditorMetrics.t,
                ~editor: Editor.t,
                (),
              ) => {
  let theme = state.theme;

  let%hook (elementRef, setElementRef) = React.Hooks.ref(None);

  let activeBuffer = Selectors.getBufferForEditor(state, editor);

  let editorId = Editor.getId(editor);
  let buffer =
    switch (activeBuffer) {
    | Some(buffer) => buffer
    | None => Buffer.empty
    };

  let bufferId = Buffer.getId(buffer);
  let lineCount = Buffer.getNumberOfLines(buffer);

  let rulers =
    Configuration.getValue(c => c.editorRulers, state.configuration);

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
  let fontFamily = state.editorFont.fontFile;
  let fontSize =
    Configuration.getValue(c => c.editorFontSize, state.configuration);

  let iFontHeight = int_of_float(fontHeight +. 0.5);
  let indentation =
    switch (Buffer.getIndentation(buffer)) {
    | Some(v) => v
    | None => IndentationSettings.default
    };

  let leftVisibleColumn = Editor.getLeftVisibleColumn(editor, metrics);
  let topVisibleLine = Editor.getTopVisibleLine(editor, metrics);
  let bottomVisibleLine = Editor.getBottomVisibleLine(editor, metrics);

  let cursorPosition = Editor.getPrimaryCursor(editor);

  let cursorLine = Index.toZeroBased(cursorPosition.line);

  let (cursorOffset, cursorCharacterWidth) =
    if (lineCount > 0 && cursorLine < lineCount) {
      let cursorStr = Buffer.getLine(buffer, cursorLine);

      let (cursorOffset, width) =
        BufferViewTokenizer.getCharacterPositionAndWidth(
          ~indentation,
          cursorStr,
          Index.toZeroBased(cursorPosition.column),
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

  let cursorPixelY =
    int_of_float(
      fontHeight
      *. float_of_int(Index.toZeroBased(cursorPosition.line))
      -. editor.scrollY
      +. 0.5,
    );

  let cursorPixelX =
    int_of_float(
      lineNumberWidth
      +. fontWidth
      *. float_of_int(cursorOffset)
      -. editor.scrollX
      +. 0.5,
    );

  let cursorStyle =
    Style.[
      position(`Absolute),
      top(cursorPixelY),
      left(cursorPixelX),
      height(iFontHeight),
      width(cursorWidth),
      backgroundColor(Colors.white),
    ];

  let isMinimapShown =
    Configuration.getValue(c => c.editorMinimapEnabled, state.configuration);

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
    Selectors.getConfigurationValue(state, buffer, c => c.editorMatchBrackets);

  let matchingPairs =
    !matchingPairsEnabled
      ? None
      : BufferHighlights.getMatchingPair(bufferId, state.bufferHighlights);

  let lineCount = Buffer.getNumberOfLines(buffer);
  let getTokensForLine =
      (~ignoreMatchingPairs=false, ~selection=None, startIndex, endIndex, i) =>
    if (i >= lineCount) {
      [];
    } else {
      let line = Buffer.getLine(buffer, i);

      let idx = Index.fromZeroBased(i);
      let highlights =
        BufferHighlights.getHighlightsByLine(
          ~bufferId,
          ~line=idx,
          state.bufferHighlights,
        );

      let isActiveLine = i == cursorLine;
      let defaultBackground =
        isActiveLine
          ? theme.editorLineHighlightBackground : theme.editorBackground;

      let matchingPairIndex =
        switch (matchingPairs) {
        | None => None
        | Some((startPos, endPos)) when !ignoreMatchingPairs =>
          if (Index.toZeroBased(startPos.line) == i) {
            Some(Index.toZeroBased(startPos.column));
          } else if (Index.toZeroBased(endPos.line) == i) {
            Some(Index.toZeroBased(endPos.column));
          } else {
            None;
          }
        | _ => None
        };

      let tokenColors =
        BufferSyntaxHighlights.getTokens(
          bufferId,
          Index.fromZeroBased(i),
          state.bufferSyntaxHighlights,
        );

      let colorizer =
        BufferLineColorizer.create(
          ~startIndex,
          ~endIndex,
          ~defaultBackgroundColor=defaultBackground,
          ~defaultForegroundColor=theme.editorForeground,
          ~selectionHighlights=selection,
          ~selectionColor=theme.editorSelectionBackground,
          ~matchingPair=matchingPairIndex,
          ~searchHighlights=highlights,
          ~searchHighlightColor=theme.editorFindMatchBackground,
          tokenColors,
        );

      BufferViewTokenizer.tokenize(
        ~startIndex,
        ~endIndex,
        line,
        IndentationSettings.default,
        colorizer,
      );
    };

  let getTokenAtPosition = (~startIndex, ~endIndex, position: Location.t) => {
    let lineNumber = position.line |> Index.toZeroBased;
    let index = position.column |> Index.toZeroBased;

    getTokensForLine(
      ~ignoreMatchingPairs=true,
      startIndex,
      endIndex,
      lineNumber,
    )
    |> List.filter((token: BufferViewTokenizer.t) => {
         let tokenStart = token.startPosition |> Index.toZeroBased;
         let tokenEnd = token.endPosition |> Index.toZeroBased;
         index >= tokenStart && index < tokenEnd;
       })
    |> Utility.Option.of_list;
  };

  let style =
    Style.[
      backgroundColor(theme.editorBackground),
      color(theme.editorForeground),
      flexGrow(1),
    ];

  let onDimensionsChanged =
      ({width, height}: NodeEvents.DimensionsChangedEventParams.t) => {
    GlobalContext.current().notifyEditorSizeChanged(
      ~editorGroupId=editorGroup.editorGroupId,
      ~width,
      ~height,
      (),
    );
  };

  let bufferPixelWidth =
    layout.lineNumberWidthInPixels +. layout.bufferWidthInPixels;

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
      backgroundColor(theme.scrollbarSliderBackground),
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
    let () =
      GlobalContext.current().editorScrollDelta(
        ~editorId,
        ~deltaY=wheelEvent.deltaY *. (-50.),
        (),
      );
    ();
  };

  let scrollMinimap = (wheelEvent: NodeEvents.mouseWheelEventParams) => {
    let () =
      GlobalContext.current().editorScrollDelta(
        ~editorId,
        ~deltaY=wheelEvent.deltaY *. (-150.),
        (),
      );
    ();
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

  let completions = () =>
    Completions.isActive(state.completions)
      ? <CompletionsView
          x=cursorPixelX
          y=cursorPixelY
          lineHeight=fontHeight
          state
        />
      : React.empty;

  let hoverElements =
    isActiveSplit
      ? <View style={Styles.bufferViewOverlay(bufferPixelWidth)}>
          <HoverView x=cursorPixelX y=cursorPixelY state />
          <completions />
        </View>
      : React.empty;

  /* TODO: Selection! */
  /*let editorMouseDown = (evt: NodeEvents.mouseButtonEventParams) => {
    };*/

  let editorMouseUp = (evt: NodeEvents.mouseButtonEventParams) => {
    switch (elementRef) {
    | None => ()
    | Some(r) =>
      let rect = r#getBoundingBox() |> Revery.Math.Rectangle.ofBoundingBox;

      let relY = evt.mouseY -. Revery.Math.Rectangle.getY(rect);
      let relX = evt.mouseX -. Revery.Math.Rectangle.getX(rect);

      let numberOfLines = Buffer.getNumberOfLines(buffer);
      let (line, col) =
        Editor.pixelPositionToLineColumn(
          editor,
          metrics,
          relX -. lineNumberWidth,
          relY,
        );

      if (line < numberOfLines) {
        Log.debug(() =>
          "EditorSurface - editorMouseUp: topVisibleLine is "
          ++ string_of_int(topVisibleLine)
        );
        Log.debug(() =>
          "EditorSurface - editorMouseUp: setPosition ("
          ++ string_of_int(line + 1)
          ++ ", "
          ++ string_of_int(col)
          ++ ")"
        );
        let cursor =
          Vim.Cursor.create(
            ~line=Index.fromOneBased(line + 1),
            ~column=Index.fromZeroBased(col),
          );

        /*GlobalContext.current().dispatch(
            Actions.EditorScrollToLine(editorId, topVisibleLine),
          );
          GlobalContext.current().dispatch(
            Actions.EditorScrollToColumn(editorId, leftVisibleColumn),
          );*/
        GlobalContext.current().dispatch(
          Actions.EditorCursorMove(editorId, [cursor]),
        );
      };
    };
  };

  <View style ref={node => setElementRef(Some(node))} onDimensionsChanged>
    <View
      style={Styles.bufferViewClipped(bufferPixelWidth)}
      onMouseUp=editorMouseUp
      onMouseWheel=scrollSurface>
      <OpenGL
        style={Styles.bufferViewClipped(bufferPixelWidth)}
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
              *. float_of_int(Index.toZeroBased(cursorPosition.line))
              -. editor.scrollY,
            ~height=fontHeight,
            ~width=float_of_int(metrics.pixelWidth) -. lineNumberWidth,
            ~color=theme.editorLineHighlightBackground,
            (),
          );

          /* Draw configured rulers */
          let renderRuler = ruler =>
            Shapes.drawRect(
              ~transform,
              ~x=fst(bufferPositionToPixel(0, ruler)),
              ~y=0.0,
              ~height=float_of_int(metrics.pixelHeight),
              ~width=float_of_int(1),
              ~color=theme.editorRulerForeground,
              (),
            );

          List.iter(renderRuler, rulers);

          let renderUnderline = (~offset=0., ~color=Colors.black, r: Range.t) =>
            {let halfOffset = offset /. 2.0;
             let line = Index.toZeroBased(r.start.line);
             let start = Index.toZeroBased(r.start.column);
             let endC = Index.toZeroBased(r.stop.column);

             let text = Buffer.getLine(buffer, line);
             let (startOffset, _) =
               BufferViewTokenizer.getCharacterPositionAndWidth(
                 ~indentation,
                 ~viewOffset=leftVisibleColumn,
                 text,
                 start,
               );
             let (endOffset, _) =
               BufferViewTokenizer.getCharacterPositionAndWidth(
                 ~indentation,
                 ~viewOffset=leftVisibleColumn,
                 text,
                 endC,
               );

             Shapes.drawRect(
               ~transform,
               ~x=
                 lineNumberWidth
                 +. float_of_int(startOffset)
                 *. fontWidth
                 -. halfOffset,
               ~y=
                 fontHeight
                 *. float_of_int(Index.toZeroBased(r.start.line))
                 -. editor.scrollY
                 -. halfOffset
                 +. (fontHeight -. 2.),
               ~height=1.,
               ~width=
                 offset
                 +. max(float_of_int(endOffset - startOffset), 1.0)
                 *. fontWidth,
               ~color,
               (),
             )};

          let renderRange = (~offset=0., ~color=Colors.black, r: Range.t) =>
            {let halfOffset = offset /. 2.0;
             let line = Index.toZeroBased(r.start.line);
             let start = Index.toZeroBased(r.start.column);
             let endC = Index.toZeroBased(r.stop.column);

             let lines = Buffer.getNumberOfLines(buffer);
             if (line < lines) {
               let text = Buffer.getLine(buffer, line);
               let (startOffset, _) =
                 BufferViewTokenizer.getCharacterPositionAndWidth(
                   ~indentation,
                   ~viewOffset=leftVisibleColumn,
                   text,
                   start,
                 );
               let (endOffset, _) =
                 BufferViewTokenizer.getCharacterPositionAndWidth(
                   ~indentation,
                   ~viewOffset=leftVisibleColumn,
                   text,
                   endC,
                 );

               Shapes.drawRect(
                 ~transform,
                 ~x=
                   lineNumberWidth
                   +. float_of_int(startOffset)
                   *. fontWidth
                   -. halfOffset,
                 ~y=
                   fontHeight
                   *. float_of_int(Index.toZeroBased(r.start.line))
                   -. editor.scrollY
                   -. halfOffset,
                 ~height=fontHeight +. offset,
                 ~width=
                   offset
                   +. max(float_of_int(endOffset - startOffset), 1.0)
                   *. fontWidth,
                 ~color,
                 (),
               );
             }};

          ImmediateList.render(
            ~scrollY,
            ~rowHeight,
            ~height=float_of_int(height),
            ~count,
            ~render=
              (item, _offset) => {
                let index = Index.fromZeroBased(item);
                let renderDiagnostics = (d: Diagnostic.t) =>
                  renderUnderline(~color=Colors.red, d.range);

                /* Draw error markers */
                switch (IntMap.find_opt(item, diagnostics)) {
                | None => ()
                | Some(v) => List.iter(renderDiagnostics, v)
                };

                switch (Hashtbl.find_opt(selectionRanges, index)) {
                | None => ()
                | Some(v) =>
                  List.iter(
                    renderRange(~color=theme.editorSelectionBackground),
                    v,
                  )
                };

                /* Draw match highlights */
                let matchColor = theme.editorSelectionBackground;
                switch (matchingPairs) {
                | None => ()
                | Some((startPos, endPos)) =>
                  renderRange(
                    ~offset=0.0,
                    ~color=matchColor,
                    Range.{start: startPos, stop: startPos},
                  );
                  renderRange(
                    ~offset=0.0,
                    ~color=matchColor,
                    Range.{start: endPos, stop: endPos},
                  );
                };

                /* Draw search highlights */
                BufferHighlights.getHighlightsByLine(
                  ~bufferId,
                  ~line=index,
                  state.bufferHighlights,
                )
                |> List.iter(r =>
                     renderRange(
                       ~offset=2.0,
                       ~color=theme.editorFindMatchBackground,
                       r,
                     )
                   );
              },
            (),
          );

          if (Definition.isAvailable(
                bufferId,
                cursorPosition,
                state.definition,
              )) {
            let () =
              getTokenAtPosition(
                ~startIndex=leftVisibleColumn,
                ~endIndex=leftVisibleColumn + layout.bufferWidthInCharacters,
                cursorPosition,
              )
              |> Utility.Option.iter((token: BufferViewTokenizer.t) => {
                   let range =
                     Range.{
                       start:
                         Location.{
                           line: cursorPosition.line,
                           column: token.startPosition,
                         },
                       stop:
                         Location.{
                           line: cursorPosition.line,
                           column: token.endPosition,
                         },
                     };
                   let () = renderUnderline(~color=token.color, range);
                   ();
                 });
            ();
          };

          ImmediateList.render(
            ~scrollY,
            ~rowHeight,
            ~height=float_of_int(height),
            ~count,
            ~render=
              (item, offset) => {
                let index = Index.fromZeroBased(item);
                let selectionRange =
                  switch (Hashtbl.find_opt(selectionRanges, index)) {
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
                    fontFamily,
                    fontSize,
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
              ~color=theme.editorLineNumberBackground,
              (),
            );

            ImmediateList.render(
              ~scrollY,
              ~rowHeight,
              ~height=float_of_int(height),
              ~count,
              ~render=
                (item, offset) => {
                  let _ =
                    renderLineNumber(
                      fontFamily,
                      fontSize,
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
                ~cursorLine=Index.toZeroBased(cursorPosition.line),
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
      <Opacity opacity=cursorOpacity> <View style=cursorStyle /> </Opacity>
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
    hoverElements
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
  </View>;
};
