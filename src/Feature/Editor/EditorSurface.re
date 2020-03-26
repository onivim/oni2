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

module Colors = {
  include Feature_Theme.Colors;
  include Editor;
};

module Constants = {
  include Constants;

  let diffMarkersMaxLineCount = 2000;
  let diffMarkerWidth = 3.;
  let gutterMargin = 3.;
};

module Styles = {
  open Style;

  let container = (~theme) => [
    backgroundColor(Colors.background.from(theme)),
    color(Colors.foreground.from(theme)),
    flexGrow(1),
  ];

  let verticalScrollBar = (~theme) =>
    Style.[
      position(`Absolute),
      top(0),
      right(0),
      width(Constants.scrollBarThickness),
      backgroundColor(Colors.ScrollbarSlider.background.from(theme)),
      bottom(0),
    ];
};

let minimap =
    (
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
        ~theme,
        ~matchingPairs,
        ~bufferSyntaxHighlights,
        0,
        bufferWidthInCharacters,
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

let make =
    (
      ~buffer,
      ~onDimensionsChanged,
      ~isActiveSplit: bool,
      ~metrics: EditorMetrics.t,
      ~editor: Editor.t,
      ~theme,
      ~editorFont: Service_Font.font,
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
  let lineCount = Buffer.getNumberOfLines(buffer);

  let leftVisibleColumn = Editor.getLeftVisibleColumn(editor, metrics);
  let topVisibleLine = Editor.getTopVisibleLine(editor, metrics);
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
      theme
      scrollY={editor.scrollY}
      lineHeight={metrics.lineHeight}
      count=lineCount
      editorFont
      cursorLine={Index.toZeroBased(cursorPosition.line)}
      diffMarkers
    />;

  <View style={Styles.container(~theme)} onDimensionsChanged>
    gutterView
    <SurfaceView
      onScroll
      buffer
      editor
      metrics
      theme
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
           theme
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
      theme
      tokenTheme
    />
    <View style={Styles.verticalScrollBar(~theme)}>
      <EditorVerticalScrollbar
        editor
        metrics
        width=Constants.scrollBarThickness
        height={metrics.pixelHeight}
        diagnostics=diagnosticsMap
        theme
        editorFont
        bufferHighlights
      />
    </View>
  </View>;
};
