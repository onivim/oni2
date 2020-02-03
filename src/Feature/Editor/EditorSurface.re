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

module Log = (val Log.withNamespace("Oni2.UI.EditorSurface"));

module Option = Utility.Option;
module FontIcon = Oni_Components.FontIcon;
module BufferHighlights = Oni_Syntax.BufferHighlights;
module Completions = Feature_LanguageSupport.Completions;
module Diagnostics = Feature_LanguageSupport.Diagnostics;
module Diagnostic = Feature_LanguageSupport.Diagnostic;
module Definition = Feature_LanguageSupport.Definition;

module Constants = {
  include Constants;

  let diffMarkersMaxLineCount = 2000;
  let diffMarkerWidth = 3.;
  let gutterMargin = 3.;
};

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
      paint: Skia.Paint.t,
      descenderHeight: float,
      fontWidth: float,
      fontHeight: float,
      lineNumber: int,
      lineNumberWidth: float,
      theme: Theme.t,
      lineSetting,
      cursorLine: int,
      yOffset: float,
      canvasContext,
    ) => {
  let isActiveLine = lineNumber == cursorLine;
  let lineNumberTextColor =
    isActiveLine
      ? theme.editorActiveLineNumberForeground
      : theme.editorLineNumberForeground;

  let yF = yOffset +. fontHeight -. descenderHeight;

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
        -. float(String.length(lineNumber))
        *. fontWidth
        /. 2.;

  Skia.Paint.setColor(paint, Color.toSkia(lineNumberTextColor));

  CanvasContext.drawText(
    ~x=lineNumberXOffset,
    ~y=yF,
    ~paint,
    ~text=lineNumber,
    canvasContext,
  );
};

let spacesPaint = Skia.Paint.make();
let renderSpaces =
    (
      ~fontWidth: float,
      ~fontHeight: float,
      ~x: float,
      ~y: float,
      ~count: int,
      ~theme: Theme.t,
      canvasContext,
    ) => {
  let i = ref(0);

  let size = 2.;
  let xOffset = fontWidth /. 2. -. 1.;
  let yOffset = fontHeight /. 2. -. 1.;

  while (i^ < count) {
    let iF = float(i^);
    let xPos = x +. fontWidth *. iF;

    Skia.Paint.setColor(
      spacesPaint,
      theme.editorWhitespaceForeground |> Color.toSkia,
    );
    CanvasContext.drawRectLtwh(
      ~left=xPos +. xOffset,
      ~top=y +. yOffset,
      ~width=size,
      ~height=size,
      ~paint=spacesPaint,
      canvasContext,
    );

    incr(i);
  };
};

let renderTokens =
    (
      tokenFont: Revery.Font.t,
      tokenPaint: Skia.Paint.t,
      descenderHeight: float,
      fontWidth: float,
      fontHeight: float,
      lineNumberWidth: float,
      theme: Theme.t,
      tokens,
      xOffset: float,
      yOffset: float,
      canvasContext,
      whitespaceSetting: ConfigurationValues.editorRenderWhitespace,
    ) => {
  let yF = yOffset;
  let xF = xOffset;

  let f = (token: BufferViewTokenizer.t) => {
    let x =
      lineNumberWidth
      +. fontWidth
      *. float(Index.toZeroBased(token.startPosition))
      -. xF;
    let y = yF +. fontHeight -. descenderHeight;

    switch (token.tokenType) {
    | Text =>
      Skia.Paint.setColor(tokenPaint, Color.toSkia(token.color));
      let shapedText =
        Revery.Font.shape(tokenFont, token.text)
        |> Revery.Font.ShapeResult.getGlyphString;

      CanvasContext.drawText(
        ~paint=tokenPaint,
        ~x,
        ~y,
        ~text=shapedText,
        canvasContext,
      );
    | Tab =>
      CanvasContext.Deprecated.drawString(
        ~x=x +. fontWidth /. 4.,
        ~y=y,
        ~color=theme.editorWhitespaceForeground,
        ~fontFamily="FontAwesome5FreeSolid.otf",
        ~fontSize=10.,
        ~text=FontIcon.codeToIcon(0xf30b),
        canvasContext,
      )
    | Whitespace =>
      renderSpaces(
        ~fontWidth,
        ~fontHeight,
        ~x,
        ~y,
        ~count=String.length(token.text),
        ~theme,
        canvasContext,
      )
    };
  };

  tokens |> WhitespaceTokenFilter.filter(whitespaceSetting) |> List.iter(f);
};

let%component make =
              (
                ~activeBuffer,
                ~onDimensionsChanged,
                ~isActiveSplit: bool,
                ~metrics: EditorMetrics.t,
                ~editor: Editor.t,
                ~theme: Theme.t,
                ~rulers=[],
                ~showLineNumbers=LineNumber.On,
                ~editorFont: EditorFont.t,
                ~fontSize=14.,
                ~mode: Vim.Mode.t,
                ~showMinimap=true,
                ~showMinimapSlider=true,
                ~maxMinimapCharacters=50,
                ~matchingPairsEnabled=true,
                ~bufferHighlights,
                ~bufferSyntaxHighlights,
                ~onScroll,
                ~diagnostics,
                ~completions,
                ~tokenTheme,
                ~hoverDelay=Time.ms(1000),
                ~isHoverEnabled=true,
                ~onCursorChange,
                ~definition,
                ~shouldRenderWhitespace=ConfigurationValues.None,
                ~shouldRenderIndentGuides=false,
                ~shouldHighlightActiveIndentGuides=false,
                (),
              ) => {
  let%hook (elementRef, setElementRef) = React.Hooks.ref(None);

  let buffer =
    switch (activeBuffer) {
    | Some(buffer) => buffer
    | None => Buffer.empty
    };

  let bufferId = Buffer.getId(buffer);
  let lineCount = Buffer.getNumberOfLines(buffer);

  let lineNumberWidth =
    showLineNumbers != LineNumber.Off
      ? LineNumber.getLineNumberPixelWidth(
          ~lines=lineCount,
          ~fontPixelWidth=editorFont.measuredWidth,
          (),
        )
      : 0.0;

  let gutterWidth =
    lineNumberWidth +. Constants.diffMarkerWidth +. Constants.gutterMargin;

  let fontHeight = editorFont.measuredHeight;
  let fontWidth = editorFont.measuredWidth;
  let fontFamily = editorFont.fontFile;
  let descenderHeight = editorFont.descenderHeight;

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
      let cursorLine = Buffer.getLine(cursorLine, buffer);

      let (cursorOffset, width) =
        BufferViewTokenizer.getCharacterPositionAndWidth(
          cursorLine,
          Index.toZeroBased(cursorPosition.column),
        );
      (cursorOffset, width);
    } else {
      (0, 1);
    };

  let bufferPositionToPixel = (line, char) => {
    let x = float(char) *. fontWidth -. editor.scrollX +. gutterWidth;
    let y = float(line) *. fontHeight -. editor.scrollY;
    (x, y);
  };

  let fullCursorWidth = cursorCharacterWidth * int_of_float(fontWidth);

  let cursorWidth =
    switch (mode, isActiveSplit) {
    | (Insert, true) => 2
    | _ => fullCursorWidth
    };

  let cursorOpacity = isActiveSplit ? 0.5 : 0.25;

  let cursorPixelY =
    int_of_float(
      fontHeight
      *. float(Index.toZeroBased(cursorPosition.line))
      -. editor.scrollY
      +. 0.5,
    );

  let cursorPixelX =
    int_of_float(
      gutterWidth +. fontWidth *. float(cursorOffset) -. editor.scrollX +. 0.5,
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

  let layout =
    EditorLayout.getLayout(
      ~showLineNumbers=showLineNumbers != LineNumber.Off,
      ~maxMinimapCharacters,
      ~pixelWidth=float(metrics.pixelWidth),
      ~pixelHeight=float(metrics.pixelHeight),
      ~isMinimapShown=showMinimap,
      ~characterWidth=editorFont.measuredWidth,
      ~characterHeight=editorFont.measuredHeight,
      ~bufferLineCount=lineCount,
      (),
    );

  let matchingPairs =
    !matchingPairsEnabled
      ? None : BufferHighlights.getMatchingPair(bufferId, bufferHighlights);

  let lineCount = Buffer.getNumberOfLines(buffer);
  let getTokensForLine =
      (~ignoreMatchingPairs=false, ~selection=None, startIndex, endIndex, i) =>
    if (i >= lineCount) {
      [];
    } else {
      let line = Buffer.getLine(i, buffer);

      let idx = Index.fromZeroBased(i);
      let highlights =
        BufferHighlights.getHighlightsByLine(
          ~bufferId,
          ~line=idx,
          bufferHighlights,
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
          bufferSyntaxHighlights,
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

      BufferViewTokenizer.tokenize(~startIndex, ~endIndex, line, colorizer);
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
    |> Utility.OptionEx.of_list;
  };

  let style =
    Style.[
      backgroundColor(theme.editorBackground),
      color(theme.editorForeground),
      flexGrow(1),
    ];

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
      left(int_of_float(bufferPixelWidth +. float(minimapPixelWidth))),
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

  let scrollSurface = (wheelEvent: NodeEvents.mouseWheelEventParams) =>
    onScroll(wheelEvent.deltaY *. (-50.));

  let scrollMinimap = (wheelEvent: NodeEvents.mouseWheelEventParams) =>
    onScroll(wheelEvent.deltaY *. (-150.));

  let diagnosticsMap =
    switch (activeBuffer) {
    | Some(b) => Diagnostics.getDiagnosticsMap(diagnostics, b)
    | None => IntMap.empty
    };
  let ranges = Selection.getRanges(editor.selection, buffer);
  let selectionRanges = Range.toHash(ranges);

  let diffMarkers =
    lineCount < Constants.diffMarkersMaxLineCount
      ? EditorDiffMarkers.generate(buffer) : None;

  let minimapLayout =
    showMinimap
      ? <View style=minimapViewStyle onMouseWheel=scrollMinimap>
          <Minimap
            editor
            width={layout.minimapWidthInPixels}
            height={metrics.pixelHeight}
            count=lineCount
            diagnostics=diagnosticsMap
            metrics
            getTokensForLine={getTokensForLine(
              0,
              layout.bufferWidthInCharacters,
            )}
            selection=selectionRanges
            showSlider=showMinimapSlider
            onScroll
            theme
            bufferHighlights
            diffMarkers
          />
        </View>
      : React.empty;

  let completions = () =>
    Completions.isActive(completions)
      ? <CompletionsView
          x=cursorPixelX
          y=cursorPixelY
          lineHeight=fontHeight
          theme
          tokenTheme
          editorFont
          completions
        />
      : React.empty;

  let hoverElements =
    isActiveSplit
      ? <View style={Styles.bufferViewOverlay(bufferPixelWidth)}>
          <HoverView
            x=cursorPixelX
            y=cursorPixelY
            delay=hoverDelay
            isEnabled=isHoverEnabled
            theme
            editorFont
            diagnostics
            editor
            buffer
            mode
          />
          <completions />
        </View>
      : React.empty;

  /* TODO: Selection! */
  /*let editorMouseDown = (evt: NodeEvents.mouseButtonEventParams) => {
    };*/

  let editorMouseUp = (evt: NodeEvents.mouseButtonEventParams) => {
    Log.trace("editorMouseUp");

    switch (elementRef) {
    | None => ()
    | Some(r) =>
      let rect = r#getBoundingBox();
      let (minX, minY, _, _) = Revery.Math.BoundingBox2d.getBounds(rect);

      let relY = evt.mouseY -. minY;
      let relX = evt.mouseX -. minX;

      let numberOfLines = Buffer.getNumberOfLines(buffer);
      let (line, col) =
        Editor.pixelPositionToLineColumn(
          editor,
          metrics,
          relX -. gutterWidth,
          relY,
        );

      if (line < numberOfLines && line >= 0 && col >= 0) {
        Log.tracef(m => m("  topVisibleLine is %i", topVisibleLine));
        Log.tracef(m => m("  setPosition (%i, %i)", line + 1, col));

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
        onCursorChange(cursor);
      };
    };
  };

  <View style ref={node => setElementRef(Some(node))} onDimensionsChanged>
    <View
      style={Styles.bufferViewClipped(bufferPixelWidth)}
      onMouseUp=editorMouseUp
      onMouseWheel=scrollSurface>
      <Canvas
        style={Styles.bufferViewClipped(bufferPixelWidth)}
        render={canvasContext => {
          let fontMaybe =
            Revery.Font.load(fontFamily) |> Utility.Result.to_option;

          let lineNumberPaint =
            fontMaybe
            |> Utility.Option.map(font => {
                 let lineNumberPaint = Skia.Paint.make();
                 Skia.Paint.setTextEncoding(lineNumberPaint, Utf8);
                 Skia.Paint.setAntiAlias(lineNumberPaint, true);
                 Skia.Paint.setLcdRenderText(lineNumberPaint, true);
                 Skia.Paint.setTextSize(lineNumberPaint, fontSize);
                 Skia.Paint.setTypeface(
                   lineNumberPaint,
                   Revery.Font.getSkiaTypeface(font),
                 );
                 lineNumberPaint;
               });

          let tokenPaint =
            fontMaybe
            |> Utility.Option.map(font => {
                 let paint = Skia.Paint.make();
                 Skia.Paint.setTextEncoding(paint, GlyphId);
                 Skia.Paint.setAntiAlias(paint, true);
                 Skia.Paint.setLcdRenderText(paint, true);
                 Skia.Paint.setTextSize(paint, fontSize);
                 Skia.Paint.setTypeface(
                   paint,
                   Revery.Font.getSkiaTypeface(font),
                 );
                 (font, paint);
               });

          let rectPaint = Skia.Paint.make();

          let count = lineCount;
          let height = metrics.pixelHeight;
          let rowHeight = metrics.lineHeight;
          let scrollY = editor.scrollY;

          /* Draw background for cursor line */
          Skia.Paint.setColor(
            rectPaint,
            theme.editorLineHighlightBackground |> Color.toSkia,
          );
          CanvasContext.drawRectLtwh(
            ~left=gutterWidth,
            ~top=
              fontHeight
              *. float(Index.toZeroBased(cursorPosition.line))
              -. editor.scrollY,
            ~height=fontHeight,
            ~width=float(metrics.pixelWidth) -. gutterWidth,
            ~paint=rectPaint,
            canvasContext,
          );

          /* Draw configured rulers */
          let renderRuler = ruler =>
            {Skia.Paint.setColor(
               rectPaint,
               theme.editorRulerForeground |> Color.toSkia,
             );
             CanvasContext.drawRectLtwh(
               ~left=fst(bufferPositionToPixel(0, ruler)),
               ~top=0.0,
               ~height=float(metrics.pixelHeight),
               ~width=1.,
               ~paint=rectPaint,
               canvasContext,
             )};

          List.iter(renderRuler, rulers);

          let renderUnderline = (~offset=0., ~color=Colors.black, r: Range.t) =>
            {let halfOffset = offset /. 2.0;
             let line = Index.toZeroBased(r.start.line);
             let start = Index.toZeroBased(r.start.column);
             let endC = Index.toZeroBased(r.stop.column);

             let text = Buffer.getLine(line, buffer);
             let (startOffset, _) =
               BufferViewTokenizer.getCharacterPositionAndWidth(
                 ~viewOffset=leftVisibleColumn,
                 text,
                 start,
               );
             let (endOffset, _) =
               BufferViewTokenizer.getCharacterPositionAndWidth(
                 ~viewOffset=leftVisibleColumn,
                 text,
                 endC,
               );

             Skia.Paint.setColor(rectPaint, color |> Color.toSkia);
             CanvasContext.drawRectLtwh(
               ~left=
                 gutterWidth +. float(startOffset) *. fontWidth -. halfOffset,
               ~top=
                 fontHeight
                 *. float(Index.toZeroBased(r.start.line))
                 -. editor.scrollY
                 -. halfOffset
                 +. (fontHeight -. 2.),
               ~height=1.,
               ~width=
                 offset
                 +. max(float(endOffset - startOffset), 1.0)
                 *. fontWidth,
               ~paint=rectPaint,
               canvasContext,
             )};

          let renderRange = (~offset=0., ~color=Colors.black, r: Range.t) =>
            {let halfOffset = offset /. 2.0;
             let line = Index.toZeroBased(r.start.line);
             let start = Index.toZeroBased(r.start.column);
             let endC = Index.toZeroBased(r.stop.column);

             let lines = Buffer.getNumberOfLines(buffer);
             if (line < lines) {
               let text = Buffer.getLine(line, buffer);
               let (startOffset, _) =
                 BufferViewTokenizer.getCharacterPositionAndWidth(
                   ~viewOffset=leftVisibleColumn,
                   text,
                   start,
                 );
               let (endOffset, _) =
                 BufferViewTokenizer.getCharacterPositionAndWidth(
                   ~viewOffset=leftVisibleColumn,
                   text,
                   endC,
                 );

               Skia.Paint.setColor(rectPaint, color |> Color.toSkia);
               CanvasContext.drawRectLtwh(
                 ~left=
                   gutterWidth
                   +. float(startOffset)
                   *. fontWidth
                   -. halfOffset,
                 ~top=
                   fontHeight
                   *. float(Index.toZeroBased(r.start.line))
                   -. editor.scrollY
                   -. halfOffset,
                 ~height=fontHeight +. offset,
                 ~width=
                   offset
                   +. max(float(endOffset - startOffset), 1.0)
                   *. fontWidth,
                 ~paint=rectPaint,
                 canvasContext,
               );
             }};

          ImmediateList.render(
            ~scrollY,
            ~rowHeight,
            ~height=float(height),
            ~count,
            ~render=
              (item, _offset) => {
                let index = Index.fromZeroBased(item);
                let renderDiagnostics = (d: Diagnostic.t) =>
                  renderUnderline(~color=Colors.red, d.range);

                /* Draw error markers */
                switch (IntMap.find_opt(item, diagnosticsMap)) {
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
                  bufferHighlights,
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

          if (Definition.isAvailable(bufferId, cursorPosition, definition)) {
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

          tokenPaint
          |> Utility.Option.iter(((font, paint)) => {
               ImmediateList.render(
                 ~scrollY,
                 ~rowHeight,
                 ~height=float(height),
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
                         font,
                         paint,
                         descenderHeight,
                         fontWidth,
                         fontHeight,
                         gutterWidth,
                         theme,
                         tokens,
                         editor.scrollX,
                         offset,
                         canvasContext,
                         shouldRenderWhitespace,
                       );
                     ();
                   },
                 (),
               )
             });

          /* Draw background for line numbers */
          if (showLineNumbers != LineNumber.Off) {
            Skia.Paint.setColor(
              rectPaint,
              theme.editorLineNumberBackground |> Color.toSkia,
            );
            CanvasContext.drawRectLtwh(
              ~left=0.,
              ~top=0.,
              ~width=lineNumberWidth,
              ~height=float(height),
              ~paint=rectPaint,
              canvasContext,
            );

            lineNumberPaint
            |> Utility.Option.iter(paint => {
                 ImmediateList.render(
                   ~scrollY,
                   ~rowHeight,
                   ~height=float(height),
                   ~count,
                   ~render=
                     (item, offset) => {
                       renderLineNumber(
                         paint,
                         descenderHeight,
                         fontWidth,
                         fontHeight,
                         item,
                         lineNumberWidth,
                         theme,
                         showLineNumbers,
                         cursorLine,
                         offset,
                         canvasContext,
                       )
                     },
                   (),
                 )
               });
          };

          Option.iter(
            EditorDiffMarkers.render(
              ~scrollY,
              ~rowHeight,
              ~x=lineNumberWidth,
              ~height=float(height),
              ~width=Constants.diffMarkerWidth,
              ~count,
              ~canvasContext,
              ~theme,
            ),
            diffMarkers,
          );

          if (shouldRenderIndentGuides) {
            switch (activeBuffer) {
            | None => ()
            | Some(buffer) =>
              IndentLineRenderer.render(
                ~canvasContext,
                ~buffer,
                ~startLine=topVisibleLine - 1,
                ~endLine=bottomVisibleLine + 1,
                ~lineHeight=fontHeight,
                ~fontWidth,
                ~cursorLine=Index.toZeroBased(cursorPosition.line),
                ~theme,
                ~indentationSettings=indentation,
                ~bufferPositionToPixel,
                ~showActive=shouldHighlightActiveIndentGuides,
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
          metrics
          width={int_of_float(layout.bufferWidthInPixels)}
          theme
        />
      </View>
    </View>
    minimapLayout
    hoverElements
    <View style=verticalScrollBarStyle>
      <EditorVerticalScrollbar
        editor
        metrics
        width={Constants.default.scrollBarThickness}
        height={metrics.pixelHeight}
        diagnostics=diagnosticsMap
        theme
        editorFont
        bufferHighlights
      />
    </View>
  </View>;
};
