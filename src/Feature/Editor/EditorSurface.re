/*
 * EditorSurface.re
 *
 * Component that handles rendering of the 'editor surface' -
 * the view of the buffer in the window.
 */

open EditorCoreTypes;
open Revery.UI;

open Oni_Core;

open Helpers;

module Log = (val Log.withNamespace("Oni2.UI.EditorSurface"));
module Config = EditorConfiguration;

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

  let container = (~colors: Colors.t) => [
    backgroundColor(colors.editorBackground),
    color(colors.editorForeground),
    flexGrow(1),
  ];

  let verticalScrollBar = (~colors: Colors.t) => [
    position(`Absolute),
    top(0),
    right(0),
    width(Constants.scrollBarThickness),
    backgroundColor(colors.scrollbarSliderBackground),
    bottom(0),
  ];
};

let minimap =
    (
      ~buffer,
      ~bufferHighlights,
      ~cursorPosition: Location.t,
      ~colors,
      ~matchingPairs,
      ~bufferSyntaxHighlights,
      ~selectionRanges,
      ~showMinimapSlider,
      ~onScroll,
      ~editor,
      ~diffMarkers,
      ~metrics: EditorMetrics.t,
      ~diagnosticsMap,
      ~bufferWidthInCharacters,
      ~minimapWidthInPixels,
      (),
    ) => {
  let minimapPixelWidth = minimapWidthInPixels + Constants.minimapPadding * 2;
  let style =
    Style.[
      position(`Absolute),
      overflow(`Hidden),
      top(0),
      right(Constants.scrollBarThickness),
      width(minimapPixelWidth),
      bottom(0),
    ];
  let onMouseWheel = (wheelEvent: NodeEvents.mouseWheelEventParams) =>
    onScroll(wheelEvent.deltaY *. (-150.));

  <View style onMouseWheel>
    <Minimap
      editor
      width=minimapPixelWidth
      height={metrics.pixelHeight}
      count={Buffer.getNumberOfLines(buffer)}
      diagnostics=diagnosticsMap
      metrics
      getTokensForLine={getTokensForLine(
        ~buffer,
        ~bufferHighlights,
        ~cursorLine=Index.toZeroBased(cursorPosition.line),
        ~colors,
        ~matchingPairs,
        ~bufferSyntaxHighlights,
        0,
        bufferWidthInCharacters,
      )}
      selection=selectionRanges
      showSlider=showMinimapSlider
      onScroll
      colors
      bufferHighlights
      diffMarkers
    />
  </View>;
};

// VIEW

let make =
    (
      ~buffer,
      ~onDimensionsChanged,
      ~isActiveSplit: bool,
      ~metrics: EditorMetrics.t,
      ~editor: Editor.t,
      ~theme,
      ~mode: Vim.Mode.t,
      ~bufferHighlights,
      ~bufferSyntaxHighlights,
      ~onScroll,
      ~diagnostics,
      ~completions,
      ~tokenTheme,
      ~onCursorChange,
      ~definition,
      ~windowIsFocused,
      ~config,
      (),
    ) => {
  let colors = Colors.precompute(theme);
  let lineCount = Buffer.getNumberOfLines(buffer);

  let editorFont = editor.font;

  let leftVisibleColumn = Editor.getLeftVisibleColumn(editor);
  let topVisibleLine = Editor.getTopVisibleLine(editor);
  let bottomVisibleLine = Editor.getBottomVisibleLine(editor, metrics);

  let cursorPosition = Editor.getPrimaryCursor(editor);

  let layout =
    EditorLayout.getLayout(
      ~showLineNumbers=Config.lineNumbers.get(config) != `Off,
      ~maxMinimapCharacters=Config.Minimap.maxColumn.get(config),
      ~pixelWidth=float(metrics.pixelWidth),
      ~pixelHeight=float(metrics.pixelHeight),
      ~isMinimapShown=Config.Minimap.enabled.get(config),
      ~characterWidth=editorFont.measuredWidth,
      ~characterHeight=editorFont.measuredHeight,
      ~bufferLineCount=lineCount,
      (),
    );

  let matchingPairs =
    !Config.matchBrackets.get(config)
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

  let (gutterWidth, gutterView) =
    <GutterView
      showLineNumbers={Config.lineNumbers.get(config)}
      height={metrics.pixelHeight}
      colors
      scrollY={editor.scrollY}
      lineHeight={editorFont.measuredHeight}
      count=lineCount
      editorFont
      cursorLine={Index.toZeroBased(cursorPosition.line)}
      diffMarkers
    />;

  <View style={Styles.container(~colors)} onDimensionsChanged>
    gutterView
    <SurfaceView
      onScroll
      buffer
      editor
      metrics
      colors
      topVisibleLine
      onCursorChange
      cursorPosition
      editorFont
      leftVisibleColumn
      diagnosticsMap
      selectionRanges
      matchingPairs
      bufferHighlights
      definition
      bufferSyntaxHighlights
      bottomVisibleLine
      mode
      isActiveSplit
      gutterWidth
      bufferWidthInCharacters={layout.bufferWidthInCharacters}
      windowIsFocused
      config
    />
    {Config.Minimap.enabled.get(config)
       ? <minimap
           editor
           diagnosticsMap
           metrics
           buffer
           bufferHighlights
           cursorPosition
           colors
           matchingPairs
           bufferSyntaxHighlights
           selectionRanges
           showMinimapSlider={Config.Minimap.showSlider.get(config)}
           diffMarkers
           onScroll
           bufferWidthInCharacters={layout.bufferWidthInCharacters}
           minimapWidthInPixels={layout.minimapWidthInPixels}
         />
       : React.empty}
    <OverlaysView
      buffer
      isActiveSplit
      hoverDelay={Config.Hover.delay.get(config)}
      isHoverEnabled={Config.Hover.enabled.get(config)}
      diagnostics
      mode
      cursorPosition
      editor
      gutterWidth
      editorFont
      completions
      colors
      tokenTheme
    />
    <View style={Styles.verticalScrollBar(~colors)}>
      <EditorVerticalScrollbar
        editor
        metrics
        width=Constants.scrollBarThickness
        height={metrics.pixelHeight}
        diagnostics=diagnosticsMap
        colors
        editorFont
        bufferHighlights
      />
    </View>
  </View>;
};
