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

  let verticalScrollBar = [
    position(`Absolute),
    top(0),
    right(0),
    width(Constants.scrollBarThickness),
    bottom(0),
  ];

  let horizontalScrollBar = (gutterOffset, width) => [
    position(`Absolute),
    bottom(0),
    left(gutterOffset),
    Style.width(width),
    height(Constants.editorHorizontalScrollBarThickness),
  ];
};

let minimap =
    (
      ~buffer,
      ~bufferHighlights,
      ~cursorPosition: Location.t,
      ~colors,
      ~dispatch,
      ~matchingPairs,
      ~bufferSyntaxHighlights,
      ~selectionRanges,
      ~showMinimapSlider,
      ~editor,
      ~diffMarkers,
      ~diagnosticsMap,
      ~bufferWidthInCharacters,
      ~minimapWidthInPixels,
      (),
    ) => {
  let minimapPixelWidth = minimapWidthInPixels + Constants.minimapPadding * 2;
  let style =
    Style.[
      position(`Absolute),
      top(0),
      right(Constants.scrollBarThickness),
      width(minimapPixelWidth),
      bottom(0),
    ];
  let onMouseWheel = (wheelEvent: NodeEvents.mouseWheelEventParams) =>
    dispatch(
      Msg.MinimapMouseWheel({deltaWheel: wheelEvent.deltaY *. (-1.)}),
    );

  <View style onMouseWheel>
    <Minimap
      editor
      cursorPosition
      dispatch
      width=minimapPixelWidth
      height={editor.pixelHeight}
      count={Buffer.getNumberOfLines(buffer)}
      diagnostics=diagnosticsMap
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
      colors
      bufferHighlights
      diffMarkers
    />
  </View>;
};

// VIEW
let scrollSpringOptions =
  Spring.Options.create(~stiffness=310., ~damping=30., ());

let%component make =
              (
                ~dispatch,
                ~showDiffMarkers=true,
                ~backgroundColor: option(Revery.Color.t)=?,
                ~foregroundColor: option(Revery.Color.t)=?,
                ~buffer,
                ~onEditorSizeChanged,
                ~isActiveSplit: bool,
                ~editor: Editor.t,
                ~theme,
                ~mode: Vim.Mode.t,
                ~bufferHighlights,
                ~bufferSyntaxHighlights,
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

  let%hook lastDimensions = Hooks.ref(None);

  // When the editor id changes, we need to make sure we're dispatching the resized
  // event, too. The ideal fix would be to have this component 'keyed' on the `editor.editorId`
  let%hook () =
    React.Hooks.effect(
      If((!=), editor.editorId),
      () => {
        lastDimensions^
        |> Option.iter(((pixelWidth, pixelHeight)) => {
             onEditorSizeChanged(editor.editorId, pixelWidth, pixelHeight)
           });

        None;
      },
    );

  let onDimensionsChanged =
      (
        {height, width, _}: Revery.UI.NodeEvents.DimensionsChangedEventParams.t,
      ) => {
    lastDimensions := Some((width, height));
    onEditorSizeChanged(editor.editorId, width, height);
  };

  let colors =
    backgroundColor
    |> Option.map(editorBackground =>
         {...colors, gutterBackground: editorBackground, editorBackground}
       )
    |> Option.value(~default=colors);
  let colors =
    foregroundColor
    |> Option.map(editorForeground => {...colors, editorForeground})
    |> Option.value(~default=colors);

  let lineCount = Buffer.getNumberOfLines(buffer);

  let editorFont = editor.font;

  let leftVisibleColumn = Editor.getLeftVisibleColumn(editor);
  let topVisibleLine = Editor.getTopVisibleLine(editor);
  let bottomVisibleLine = Editor.getBottomVisibleLine(editor);

  let cursorPosition = Editor.getPrimaryCursor(~buffer, editor);

  let layout =
    EditorLayout.getLayout(
      ~showLineNumbers=Config.lineNumbers.get(config) != `Off,
      ~maxMinimapCharacters=Config.Minimap.maxColumn.get(config),
      ~pixelWidth=float(editor.pixelWidth),
      ~pixelHeight=float(editor.pixelHeight),
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
    lineCount < Constants.diffMarkersMaxLineCount && showDiffMarkers
      ? EditorDiffMarkers.generate(buffer) : None;

  let smoothScroll = Config.Experimental.smoothScroll.get(config);

  let%hook (scrollY, _setScrollYImmediately) =
    Hooks.spring(
      ~target=editor.scrollY,
      ~restThreshold=10.,
      ~enabled=smoothScroll,
      scrollSpringOptions,
    );
  let%hook (scrollX, _setScrollXImmediately) =
    Hooks.spring(
      ~target=editor.scrollX,
      ~restThreshold=10.,
      ~enabled=smoothScroll,
      scrollSpringOptions,
    );

  let editor = {...editor, scrollX, scrollY};

  let (gutterWidth, gutterView) =
    <GutterView
      showLineNumbers={Config.lineNumbers.get(config)}
      height={editor.pixelHeight}
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
      buffer
      editor
      colors
      dispatch
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
      bufferPixelWidth={int_of_float(layout.bufferWidthInPixels)}
      bufferWidthInCharacters={layout.bufferWidthInCharacters}
      windowIsFocused
      config
    />
    {Config.Minimap.enabled.get(config)
       ? <minimap
           editor
           diagnosticsMap
           buffer
           bufferHighlights
           cursorPosition
           colors
           dispatch
           matchingPairs
           bufferSyntaxHighlights
           selectionRanges
           showMinimapSlider={Config.Minimap.showSlider.get(config)}
           diffMarkers
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
      theme
      tokenTheme
    />
    <View style=Styles.verticalScrollBar>
      <Scrollbar.Vertical
        dispatch
        editor
        cursorPosition
        width=Constants.scrollBarThickness
        height={editor.pixelHeight}
        diagnostics=diagnosticsMap
        colors
        bufferHighlights
      />
    </View>
    <View
      style={Styles.horizontalScrollBar(
        int_of_float(gutterWidth),
        int_of_float(layout.bufferWidthInPixels),
      )}>
      <Scrollbar.Horizontal
        dispatch
        editor
        width={int_of_float(layout.bufferWidthInPixels)}
        colors
      />
    </View>
  </View>;
};
