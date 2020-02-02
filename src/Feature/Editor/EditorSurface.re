/*
 * EditorSurface.re
 *
 * Component that handles rendering of the 'editor surface' -
 * the view of the buffer in the window.
 */

open EditorCoreTypes;
open Revery;
open Revery.UI;

open Oni_Core;

open Helpers;

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

let make =
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

  <View style={Styles.container(~theme)} onDimensionsChanged>
    <GutterView
      showLineNumbers
      height={metrics.pixelHeight}
      theme
      scrollY={editor.scrollY}
      rowHeight={metrics.lineHeight}
      count=lineCount
      editorFont
      cursorLine={Index.toZeroBased(cursorPosition.line)}
      diffMarkers
    />
    <SurfaceView
      onScroll
      buffer
      editor
      metrics
      theme
      topVisibleLine
      onCursorChange
      layout
      cursorPosition
      rulers
      editorFont
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
      gutterWidth
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
    <OverlaysView
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
