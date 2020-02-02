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

module Styles = {
  open Style;

  let container = (~theme: Theme.t) => [
    backgroundColor(theme.editorBackground),
    color(theme.editorForeground),
    flexGrow(1),
  ];

  let bufferViewCommon = bufferPixelWidth => [
    position(`Absolute),
    top(0),
    left(0),
    width(int_of_float(bufferPixelWidth)),
    bottom(0),
  ];

  let bufferViewOverlay = bufferPixelWidth => [
    pointerEvents(`Ignore),
    ...bufferViewCommon(bufferPixelWidth),
  ];

  let bufferViewClipped = bufferPixelWidth => [
    overflow(`Hidden),
    ...bufferViewCommon(bufferPixelWidth),
  ];

  let verticalScrollBar = (~theme: Theme.t) =>
    Style.[
      position(`Absolute),
      top(0),
      // left(int_of_float(bufferPixelWidth +. float(minimapPixelWidth))),
      right(0),
      width(Constants.default.scrollBarThickness),
      backgroundColor(theme.scrollbarSliderBackground),
      bottom(0),
    ];
};

let bufferPositionToPixel =
    (~gutterWidth, ~editor: Editor.t, ~editorFont: EditorFont.t, line, char) => {
  let x =
    float(char) *. editorFont.measuredWidth -. editor.scrollX +. gutterWidth;
  let y = float(line) *. editorFont.measuredHeight -. editor.scrollY;
  (x, y);
};

let getTokensForLine =
    (
      ~buffer,
      ~bufferHighlights,
      ~cursorLine,
      ~theme: Theme.t,
      ~matchingPairs,
      ~bufferSyntaxHighlights,
      ~ignoreMatchingPairs=false,
      ~selection=None,
      startIndex,
      endIndex,
      i,
    ) =>
  if (i >= Buffer.getNumberOfLines(buffer)) {
    [];
  } else {
    let line = Buffer.getLine(i, buffer);

    let idx = Index.fromZeroBased(i);
    let highlights =
      BufferHighlights.getHighlightsByLine(
        ~bufferId=Buffer.getId(buffer),
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
      | Some((startPos: Location.t, endPos: Location.t))
          when !ignoreMatchingPairs =>
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
        Buffer.getId(buffer),
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

let getTokenAtPosition =
    (
      ~buffer,
      ~bufferHighlights,
      ~cursorLine,
      ~theme,
      ~matchingPairs,
      ~bufferSyntaxHighlights,
      ~startIndex,
      ~endIndex,
      position: Location.t,
    ) => {
  let lineNumber = position.line |> Index.toZeroBased;
  let index = position.column |> Index.toZeroBased;

  getTokensForLine(
    ~buffer,
    ~bufferHighlights,
    ~cursorLine,
    ~theme,
    ~matchingPairs,
    ~bufferSyntaxHighlights,
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

// PRIMITIVES

module Primitives = {
  let drawUnderline =
      (
        ~offset=0.,
        ~buffer,
        ~leftVisibleColumn,
        ~transform,
        ~gutterWidth,
        ~editor: Editor.t,
        ~editorFont: EditorFont.t,
        ~color=Colors.black,
        r: Range.t,
      ) => {
    let halfOffset = offset /. 2.0;
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

    Shapes.drawRect(
      ~transform,
      ~x=
        gutterWidth
        +. float(startOffset)
        *. editorFont.measuredWidth
        -. halfOffset,
      ~y=
        editorFont.measuredHeight
        *. float(Index.toZeroBased(r.start.line))
        -. editor.scrollY
        -. halfOffset
        +. (editorFont.measuredHeight -. 2.),
      ~height=1.,
      ~width=
        offset
        +. max(float(endOffset - startOffset), 1.0)
        *. editorFont.measuredWidth,
      ~color,
      (),
    );
  };

  let renderRange =
      (
        ~offset=0.,
        ~buffer,
        ~leftVisibleColumn,
        ~transform,
        ~gutterWidth,
        ~editor: Editor.t,
        ~editorFont: EditorFont.t,
        ~color=Colors.black,
        r: Range.t,
      ) => {
    let halfOffset = offset /. 2.0;
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

      Shapes.drawRect(
        ~transform,
        ~x=
          gutterWidth
          +. float(startOffset)
          *. editorFont.measuredWidth
          -. halfOffset,
        ~y=
          editorFont.measuredHeight
          *. float(Index.toZeroBased(r.start.line))
          -. editor.scrollY
          -. halfOffset,
        ~height=editorFont.measuredHeight +. offset,
        ~width=
          offset
          +. max(float(endOffset - startOffset), 1.0)
          *. editorFont.measuredWidth,
        ~color,
        (),
      );
    };
  };
};

// GUTTER

module Gutter = {
  let renderLineNumber =
      (
        lineNumber: int,
        lineNumberWidth: float,
        theme: Theme.t,
        editorFont: EditorFont.t,
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
          -. float(String.length(lineNumber))
          *. editorFont.measuredWidth
          /. 2.;

    Revery.Draw.Text.drawString(
      ~window=Revery.UI.getActiveWindow(),
      ~transform,
      ~x=lineNumberXOffset,
      ~y=yF,
      ~backgroundColor=theme.editorLineNumberBackground,
      ~color=lineNumberTextColor,
      ~fontFamily=editorFont.fontFile,
      ~fontSize=editorFont.fontSize,
      lineNumber,
    );
  };

  let renderLineNumbers =
      (
        ~transform,
        ~lineNumberWidth,
        ~height,
        ~theme: Theme.t,
        ~editorFont,
        ~scrollY,
        ~rowHeight,
        ~count,
        ~showLineNumbers,
        ~cursorLine,
      ) => {
    Shapes.drawRect(
      ~transform,
      ~x=0.,
      ~y=0.,
      ~width=lineNumberWidth,
      ~height=float(height),
      ~color=theme.editorLineNumberBackground,
      (),
    );

    ImmediateList.render(
      ~scrollY,
      ~rowHeight,
      ~height=float(height),
      ~count,
      ~render=
        (item, offset) => {
          renderLineNumber(
            item,
            lineNumberWidth,
            theme,
            editorFont,
            showLineNumbers,
            cursorLine,
            offset,
            transform,
          )
        },
      (),
    );
  };

  let render =
      (
        ~showLineNumbers,
        ~transform,
        ~lineNumberWidth,
        ~height,
        ~theme,
        ~editorFont,
        ~scrollY,
        ~rowHeight,
        ~count,
        ~cursorLine,
        ~diffMarkers,
      ) => {
    if (showLineNumbers != LineNumber.Off) {
      renderLineNumbers(
        ~transform,
        ~lineNumberWidth,
        ~height,
        ~theme,
        ~editorFont,
        ~scrollY,
        ~rowHeight,
        ~count,
        ~showLineNumbers,
        ~cursorLine,
      );
    };

    Option.iter(
      EditorDiffMarkers.render(
        ~scrollY,
        ~rowHeight,
        ~x=lineNumberWidth,
        ~height=float(height),
        ~width=Constants.diffMarkerWidth,
        ~count,
        ~transform,
        ~theme,
      ),
      diffMarkers,
    );
  };
};

let drawCurrentLineHighlight =
    (
      line,
      ~transform,
      ~gutterWidth,
      ~metrics: EditorMetrics.t,
      ~editor: Editor.t,
      ~lineHeight,
      ~theme: Theme.t,
    ) =>
  Shapes.drawRect(
    ~transform,
    ~x=gutterWidth,
    ~y=lineHeight *. float(Index.toZeroBased(line)) -. editor.scrollY,
    ~height=lineHeight,
    ~width=float(metrics.pixelWidth) -. gutterWidth,
    ~color=theme.editorLineHighlightBackground,
    (),
  );

let drawRuler = (x, ~transform, ~metrics: EditorMetrics.t, ~theme: Theme.t) =>
  Shapes.drawRect(
    ~transform,
    ~x,
    ~y=0.0,
    ~height=float(metrics.pixelHeight),
    ~width=1.,
    ~color=theme.editorRulerForeground,
    (),
  );

let renderRulers =
    (
      ~rulers,
      ~gutterWidth,
      ~editor,
      ~editorFont,
      ~transform,
      ~metrics,
      ~theme,
    ) =>
  rulers
  |> List.map(bufferPositionToPixel(~gutterWidth, ~editor, ~editorFont, 0))
  |> List.map(fst)
  |> List.iter(drawRuler(~transform, ~metrics, ~theme));

// CONTENT

module Content = {
  let renderLine =
      (
        ~transform,
        ~buffer,
        ~gutterWidth,
        ~leftVisibleColumn,
        ~editor: Editor.t,
        ~editorFont,
        ~theme: Theme.t,
        ~diagnosticsMap,
        ~selectionRanges,
        ~matchingPairs,
        ~bufferHighlights,
        item,
        _offset,
      ) => {
    let index = Index.fromZeroBased(item);
    let renderDiagnostics = (d: Diagnostic.t) =>
      Primitives.drawUnderline(
        ~buffer,
        ~gutterWidth,
        ~leftVisibleColumn,
        ~transform,
        ~editor,
        ~editorFont,
        ~color=Colors.red,
        d.range,
      );

    /* Draw error markers */
    switch (IntMap.find_opt(item, diagnosticsMap)) {
    | None => ()
    | Some(v) => List.iter(renderDiagnostics, v)
    };

    switch (Hashtbl.find_opt(selectionRanges, index)) {
    | None => ()
    | Some(v) =>
      List.iter(
        Primitives.renderRange(
          ~buffer,
          ~gutterWidth,
          ~leftVisibleColumn,
          ~transform,
          ~editor,
          ~editorFont,
          ~color=theme.editorSelectionBackground,
        ),
        v,
      )
    };

    /* Draw match highlights */
    let matchColor = theme.editorSelectionBackground;
    switch (matchingPairs) {
    | None => ()
    | Some((startPos, endPos)) =>
      Primitives.renderRange(
        ~buffer,
        ~gutterWidth,
        ~leftVisibleColumn,
        ~transform,
        ~editor,
        ~editorFont,
        ~color=matchColor,
        Range.{start: startPos, stop: startPos},
      );
      Primitives.renderRange(
        ~buffer,
        ~gutterWidth,
        ~leftVisibleColumn,
        ~transform,
        ~editor,
        ~editorFont,
        ~color=matchColor,
        Range.{start: endPos, stop: endPos},
      );
    };

    /* Draw search highlights */
    BufferHighlights.getHighlightsByLine(
      ~bufferId=Buffer.getId(buffer),
      ~line=index,
      bufferHighlights,
    )
    |> List.iter(
         Primitives.renderRange(
           ~buffer,
           ~editorFont,
           ~gutterWidth,
           ~leftVisibleColumn,
           ~transform,
           ~editor,
           ~offset=2.0,
           ~color=theme.editorFindMatchBackground,
         ),
       );
  };

  let renderEmbellishments =
      (
        ~scrollY,
        ~rowHeight,
        ~height,
        ~count,
        ~transform,
        ~buffer,
        ~editor,
        ~editorFont,
        ~gutterWidth,
        ~leftVisibleColumn,
        ~theme,
        ~diagnosticsMap,
        ~selectionRanges,
        ~matchingPairs,
        ~bufferHighlights,
      ) =>
    ImmediateList.render(
      ~scrollY,
      ~rowHeight,
      ~height=float(height),
      ~count,
      ~render=
        renderLine(
          ~transform,
          ~buffer,
          ~editor,
          ~editorFont,
          ~gutterWidth,
          ~leftVisibleColumn,
          ~theme,
          ~diagnosticsMap,
          ~selectionRanges,
          ~matchingPairs,
          ~bufferHighlights,
        ),
      (),
    );

  let renderDefinition =
      (
        ~leftVisibleColumn,
        ~layout: EditorLayout.t,
        ~cursorPosition: Location.t,
        ~buffer,
        ~gutterWidth,
        ~transform,
        ~editor,
        ~editorFont,
        ~bufferHighlights,
        ~theme,
        ~matchingPairs,
        ~bufferSyntaxHighlights,
      ) =>
    getTokenAtPosition(
      ~buffer,
      ~bufferHighlights,
      ~cursorLine=Index.toZeroBased(cursorPosition.line),
      ~theme,
      ~matchingPairs,
      ~bufferSyntaxHighlights,
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
         Primitives.drawUnderline(
           ~buffer,
           ~editorFont,
           ~gutterWidth,
           ~leftVisibleColumn,
           ~transform,
           ~editor,
           ~color=token.color,
           range,
         );
       });

  let renderSpaces =
      (
        ~editorFont: EditorFont.t,
        ~x: float,
        ~y: float,
        ~transform,
        ~count: int,
        ~theme: Theme.t,
        (),
      ) => {
    let i = ref(0);

    let size = 2.;
    let xOffset = editorFont.measuredWidth /. 2. -. 1.;
    let yOffset = editorFont.measuredHeight /. 2. -. 1.;

    while (i^ < count) {
      let iF = float(i^);
      let xPos = x +. editorFont.measuredWidth *. iF;

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
        editorFont: EditorFont.t,
        gutterWidth: float,
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
        gutterWidth
        +. editorFont.measuredWidth
        *. float(Index.toZeroBased(token.startPosition))
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
          ~fontFamily=editorFont.fontFile,
          ~fontSize=editorFont.fontSize,
          token.text,
        )
      | Tab =>
        Revery.Draw.Text.drawString(
          ~window=Revery.UI.getActiveWindow(),
          ~transform,
          ~x=x +. editorFont.measuredWidth /. 4.,
          ~y=y +. editorFont.measuredHeight /. 4.,
          ~backgroundColor,
          ~color=theme.editorWhitespaceForeground,
          ~fontFamily="FontAwesome5FreeSolid.otf",
          ~fontSize=10,
          FontIcon.codeToIcon(0xf30b),
        )
      | Whitespace =>
        renderSpaces(
          ~editorFont,
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

  let renderText =
      (
        ~scrollY,
        ~rowHeight,
        ~height,
        ~count,
        ~selectionRanges,
        ~buffer,
        ~bufferHighlights,
        ~cursorLine,
        ~theme,
        ~matchingPairs,
        ~bufferSyntaxHighlights,
        ~leftVisibleColumn,
        ~layout: EditorLayout.t,
        ~editorFont,
        ~gutterWidth,
        ~editor: Editor.t,
        ~transform,
        ~shouldRenderWhitespace,
      ) =>
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
              ~buffer,
              ~bufferHighlights,
              ~cursorLine,
              ~theme,
              ~matchingPairs,
              ~bufferSyntaxHighlights,
              ~selection=selectionRange,
              leftVisibleColumn,
              leftVisibleColumn + layout.bufferWidthInCharacters,
              item,
            );

          let _ =
            renderTokens(
              editorFont,
              gutterWidth,
              theme,
              tokens,
              editor.scrollX,
              offset,
              transform,
              shouldRenderWhitespace,
            );
          ();
        },
      (),
    );

  let render =
      (
        ~scrollY,
        ~rowHeight,
        ~height,
        ~count,
        ~transform,
        ~buffer,
        ~editor,
        ~editorFont,
        ~gutterWidth,
        ~leftVisibleColumn,
        ~theme,
        ~diagnosticsMap,
        ~selectionRanges,
        ~matchingPairs,
        ~bufferHighlights,
        ~cursorPosition: Location.t,
        ~definition,
        ~layout,
        ~bufferSyntaxHighlights,
        ~shouldRenderWhitespace,
      ) => {
    renderEmbellishments(
      ~scrollY,
      ~rowHeight,
      ~height,
      ~count,
      ~transform,
      ~buffer,
      ~editor,
      ~editorFont,
      ~gutterWidth,
      ~leftVisibleColumn,
      ~theme,
      ~diagnosticsMap,
      ~selectionRanges,
      ~matchingPairs,
      ~bufferHighlights,
    );

    if (Definition.isAvailable(
          Buffer.getId(buffer),
          cursorPosition,
          definition,
        )) {
      renderDefinition(
        ~leftVisibleColumn,
        ~layout,
        ~cursorPosition,
        ~buffer,
        ~editorFont,
        ~gutterWidth,
        ~transform,
        ~editor,
        ~bufferHighlights,
        ~theme,
        ~matchingPairs,
        ~bufferSyntaxHighlights,
      );
    };

    renderText(
      ~scrollY,
      ~rowHeight,
      ~height,
      ~count,
      ~selectionRanges,
      ~buffer,
      ~bufferHighlights,
      ~cursorLine=Index.toZeroBased(cursorPosition.line),
      ~theme,
      ~matchingPairs,
      ~bufferSyntaxHighlights,
      ~leftVisibleColumn,
      ~layout,
      ~gutterWidth,
      ~editor,
      ~editorFont,
      ~transform,
      ~shouldRenderWhitespace,
    );
  };
};

module Overlay = {
  let completionsView =
      (
        ~completions,
        ~cursorPixelX,
        ~cursorPixelY,
        ~theme,
        ~tokenTheme,
        ~editorFont: EditorFont.t,
        (),
      ) =>
    Completions.isActive(completions)
      ? <CompletionsView
          x=cursorPixelX
          y=cursorPixelY
          lineHeight={editorFont.measuredHeight}
          theme
          tokenTheme
          editorFont
          completions
        />
      : React.empty;

  let make =
      (
        ~buffer,
        ~isActiveSplit,
        ~hoverDelay,
        ~isHoverEnabled,
        ~diagnostics,
        ~mode,
        ~layout: EditorLayout.t,
        ~cursorPosition: Location.t,
        ~editor: Editor.t,
        ~gutterWidth,
        ~completions,
        ~theme,
        ~tokenTheme,
        ~editorFont: EditorFont.t,
        (),
      ) => {
    let cursorLine = Index.toZeroBased(cursorPosition.line);
    let lineCount = Buffer.getNumberOfLines(buffer);

    let bufferPixelWidth =
      layout.lineNumberWidthInPixels +. layout.bufferWidthInPixels;

    let (cursorOffset, _cursorCharacterWidth) =
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

    let cursorPixelY =
      int_of_float(
        editorFont.measuredHeight
        *. float(Index.toZeroBased(cursorPosition.line))
        -. editor.scrollY
        +. 0.5,
      );

    let cursorPixelX =
      int_of_float(
        gutterWidth
        +. editorFont.measuredWidth
        *. float(cursorOffset)
        -. editor.scrollX
        +. 0.5,
      );

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
          <completionsView
            completions
            cursorPixelX
            cursorPixelY
            theme
            tokenTheme
            editorFont
          />
        </View>
      : React.empty;
  };
};

module Cursor = {
  let make =
      (
        ~buffer,
        ~mode: Vim.Mode.t,
        ~isActiveSplit,
        ~editorFont: EditorFont.t,
        ~cursorPosition: Location.t,
        ~editor: Editor.t,
        ~gutterWidth,
        (),
      ) => {
    let cursorLine = Index.toZeroBased(cursorPosition.line);
    let lineCount = Buffer.getNumberOfLines(buffer);

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

    let fullCursorWidth =
      cursorCharacterWidth * int_of_float(editorFont.measuredWidth);

    let cursorWidth =
      switch (mode, isActiveSplit) {
      | (Insert, true) => 2
      | _ => fullCursorWidth
      };

    let cursorPixelY =
      int_of_float(
        editorFont.measuredHeight
        *. float(Index.toZeroBased(cursorPosition.line))
        -. editor.scrollY
        +. 0.5,
      );

    let cursorPixelX =
      int_of_float(
        gutterWidth
        +. editorFont.measuredWidth
        *. float(cursorOffset)
        -. editor.scrollX
        +. 0.5,
      );

    let style =
      Style.[
        position(`Absolute),
        top(cursorPixelY),
        left(cursorPixelX),
        height(int_of_float(editorFont.measuredHeight)),
        width(cursorWidth),
        backgroundColor(Colors.white),
      ];
    let cursorOpacity = isActiveSplit ? 0.5 : 0.25;

    <Opacity opacity=cursorOpacity> <View style /> </Opacity>;
  };
};

module Surface = {
  let make =
      (
        ~onScroll,
        ~elementRef,
        ~buffer,
        ~editor,
        ~metrics,
        ~gutterWidth,
        ~theme,
        ~showLineNumbers,
        ~topVisibleLine,
        ~onCursorChange,
        ~layout: EditorLayout.t,
        ~cursorPosition: Location.t,
        ~rulers,
        ~lineNumberWidth,
        ~editorFont: EditorFont.t,
        ~diffMarkers,
        ~leftVisibleColumn,
        ~diagnosticsMap,
        ~selectionRanges,
        ~matchingPairs,
        ~bufferHighlights,
        ~definition,
        ~bufferSyntaxHighlights,
        ~shouldRenderWhitespace,
        ~shouldRenderIndentGuides,
        ~bottomVisibleLine,
        ~shouldHighlightActiveIndentGuides,
        ~mode,
        ~isActiveSplit,
        (),
      ) => {
    let bufferPixelWidth =
      layout.lineNumberWidthInPixels +. layout.bufferWidthInPixels;
    let lineCount = Buffer.getNumberOfLines(buffer);
    let indentation =
      switch (Buffer.getIndentation(buffer)) {
      | Some(v) => v
      | None => IndentationSettings.default
      };

    let onMouseWheel = (wheelEvent: NodeEvents.mouseWheelEventParams) =>
      onScroll(wheelEvent.deltaY *. (-50.));

    let onMouseUp = (evt: NodeEvents.mouseButtonEventParams) => {
      Log.trace("editorMouseUp");

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
            relX -. gutterWidth,
            relY,
          );

        if (line < numberOfLines) {
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

    let horizontalScrollBarStyle =
      Style.[
        position(`Absolute),
        bottom(0),
        left(int_of_float(layout.lineNumberWidthInPixels)),
        height(Constants.default.scrollBarThickness),
        width(int_of_float(layout.bufferWidthInPixels)),
      ];

    <View
      style={Styles.bufferViewClipped(bufferPixelWidth)}
      onMouseUp
      onMouseWheel>
      <OpenGL
        style={Styles.bufferViewClipped(bufferPixelWidth)}
        render={(transform, _ctx) => {
          let count = lineCount;
          let height = metrics.pixelHeight;
          let rowHeight = metrics.lineHeight;
          let scrollY = editor.scrollY;

          drawCurrentLineHighlight(
            cursorPosition.line,
            ~transform,
            ~gutterWidth,
            ~metrics,
            ~editor,
            ~lineHeight={editorFont.measuredHeight},
            ~theme,
          );

          renderRulers(
            ~rulers,
            ~editorFont,
            ~gutterWidth,
            ~editor,
            ~transform,
            ~metrics,
            ~theme,
          );

          Gutter.render(
            ~showLineNumbers,
            ~transform,
            ~lineNumberWidth,
            ~height,
            ~theme,
            ~scrollY,
            ~rowHeight,
            ~count,
            ~editorFont,
            ~cursorLine=Index.toZeroBased(cursorPosition.line),
            ~diffMarkers,
          );

          let bufferPositionToPixel =
            bufferPositionToPixel(~gutterWidth, ~editor, ~editorFont);

          Content.render(
            ~scrollY,
            ~rowHeight,
            ~height,
            ~count,
            ~transform,
            ~buffer,
            ~editor,
            ~gutterWidth,
            ~leftVisibleColumn,
            ~theme,
            ~diagnosticsMap,
            ~selectionRanges,
            ~matchingPairs,
            ~bufferHighlights,
            ~cursorPosition,
            ~definition,
            ~layout,
            ~bufferSyntaxHighlights,
            ~editorFont,
            ~shouldRenderWhitespace,
          );

          if (shouldRenderIndentGuides) {
            IndentLineRenderer.render(
              ~transform,
              ~buffer,
              ~startLine=topVisibleLine - 1,
              ~endLine=bottomVisibleLine + 1,
              ~lineHeight=editorFont.measuredHeight,
              ~fontWidth=editorFont.measuredWidth,
              ~cursorLine=Index.toZeroBased(cursorPosition.line),
              ~theme,
              ~indentationSettings=indentation,
              ~bufferPositionToPixel,
              ~showActive=shouldHighlightActiveIndentGuides,
              (),
            );
          };
        }}
      />
      <Cursor
        buffer
        mode
        isActiveSplit
        cursorPosition
        editor
        editorFont
        gutterWidth
      />
      <View style=horizontalScrollBarStyle>
        <EditorHorizontalScrollbar
          editor
          metrics
          width={int_of_float(layout.bufferWidthInPixels)}
          theme
        />
      </View>
    </View>;
  };
};

let minimap =
    (
      ~layout: EditorLayout.t,
      ~buffer,
      ~bufferHighlights,
      ~cursorPosition: Location.t,
      ~theme,
      ~matchingPairs,
      ~bufferSyntaxHighlights,
      ~selectionRanges,
      ~showMinimapSlider,
      ~onScroll,
      ~editor,
      ~diffMarkers,
      ~metrics: EditorMetrics.t,
      ~diagnosticsMap,
      (),
    ) => {
  let bufferPixelWidth =
    layout.lineNumberWidthInPixels +. layout.bufferWidthInPixels;
  let minimapPixelWidth =
    layout.minimapWidthInPixels + Constants.default.minimapPadding * 2;
  let style =
    Style.[
      position(`Absolute),
      overflow(`Hidden),
      top(0),
      left(int_of_float(bufferPixelWidth)),
      width(minimapPixelWidth),
      bottom(0),
    ];

  let onMouseWheel = (wheelEvent: NodeEvents.mouseWheelEventParams) =>
    onScroll(wheelEvent.deltaY *. (-150.));

  <View style onMouseWheel>
    <Minimap
      editor
      width={layout.minimapWidthInPixels}
      height={metrics.pixelHeight}
      count={Buffer.getNumberOfLines(buffer)}
      diagnostics=diagnosticsMap
      metrics
      getTokensForLine={getTokensForLine(
        ~buffer,
        ~bufferHighlights,
        ~cursorLine=Index.toZeroBased(cursorPosition.line),
        ~theme,
        ~matchingPairs,
        ~bufferSyntaxHighlights,
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
  </View>;
};

// VIEW

let%component make =
              (
                ~buffer,
                ~onDimensionsChanged,
                ~isActiveSplit: bool,
                ~metrics: EditorMetrics.t,
                ~editor: Editor.t,
                ~theme: Theme.t,
                ~rulers=[],
                ~showLineNumbers=LineNumber.On,
                ~editorFont: EditorFont.t,
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

  let lineCount = Buffer.getNumberOfLines(buffer);

  let leftVisibleColumn = Editor.getLeftVisibleColumn(editor, metrics);
  let topVisibleLine = Editor.getTopVisibleLine(editor, metrics);
  let bottomVisibleLine = Editor.getBottomVisibleLine(editor, metrics);

  let cursorPosition = Editor.getPrimaryCursor(editor);

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
      ? None
      : BufferHighlights.getMatchingPair(
          Buffer.getId(buffer),
          bufferHighlights,
        );

  let diagnosticsMap = Diagnostics.getDiagnosticsMap(diagnostics, buffer);
  let selectionRanges =
    Selection.getRanges(editor.selection, buffer) |> Range.toHash;

  let diffMarkers =
    lineCount < Constants.diffMarkersMaxLineCount
      ? EditorDiffMarkers.generate(buffer) : None;

  /* TODO: Selection! */
  /*let editorMouseDown = (evt: NodeEvents.mouseButtonEventParams) => {
    };*/
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

  <View
    style={Styles.container(~theme)}
    ref={node => setElementRef(Some(node))}
    onDimensionsChanged>
    <Surface
      onScroll
      elementRef
      buffer
      editor
      metrics
      gutterWidth
      theme
      showLineNumbers
      topVisibleLine
      onCursorChange
      layout
      cursorPosition
      rulers
      editorFont
      lineNumberWidth
      diffMarkers
      leftVisibleColumn
      diagnosticsMap
      selectionRanges
      matchingPairs
      bufferHighlights
      definition
      bufferSyntaxHighlights
      shouldRenderWhitespace
      shouldRenderIndentGuides
      bottomVisibleLine
      shouldHighlightActiveIndentGuides
      mode
      isActiveSplit
    />
    {showMinimap
       ? <minimap
           editor
           diagnosticsMap
           metrics
           buffer
           bufferHighlights
           cursorPosition
           theme
           matchingPairs
           bufferSyntaxHighlights
           layout
           selectionRanges
           showMinimapSlider
           diffMarkers
           onScroll
         />
       : React.empty}
    <Overlay
      buffer
      isActiveSplit
      hoverDelay
      isHoverEnabled
      diagnostics
      mode
      layout
      cursorPosition
      editor
      gutterWidth
      editorFont
      completions
      theme
      tokenTheme
    />
    <View style={Styles.verticalScrollBar(~theme)}>
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
