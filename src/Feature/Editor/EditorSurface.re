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

  let pixelHeight = Editor.visiblePixelHeight(editor);
  let count = Editor.totalViewLines(editor);

  <View style onMouseWheel>
    <Minimap
      editor
      cursorPosition
      dispatch
      width=minimapPixelWidth
      height=pixelHeight
      count
      diagnostics=diagnosticsMap
      getTokensForLine={getTokensForLine(
        ~editor,
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
                ~renderOverlays=(~gutterWidth as _: float) => <View />,
                (),
              ) => {
  let colors = Colors.precompute(theme);

  let%hook lastDimensions = Hooks.ref(None);

  let editorId = Editor.getId(editor);

  // When the editor id changes, we need to make sure we're dispatching the resized
  // event, too. The ideal fix would be to have this component 'keyed' on the `editor.editorId`
  let%hook () =
    React.Hooks.effect(
      If((!=), editorId),
      () => {
        lastDimensions^
        |> Option.iter(((pixelWidth, pixelHeight)) => {
             onEditorSizeChanged(editorId, pixelWidth, pixelHeight)
           });

        None;
      },
    );

  let onDimensionsChanged =
      (
        {height, width, _}: Revery.UI.NodeEvents.DimensionsChangedEventParams.t,
      ) => {
    lastDimensions := Some((width, height));
    onEditorSizeChanged(editorId, width, height);
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

  let lineCount = editor |> Editor.totalViewLines;

  let editorFont = Editor.font(editor);

  let leftVisibleColumn = Editor.getLeftVisibleColumn(editor);
  let topVisibleLine = Editor.getTopVisibleLine(editor);
  let bottomVisibleLine = Editor.getBottomVisibleLine(editor);

  let cursorPosition = Editor.getPrimaryCursor(editor);

  let layout =
    Editor.getLayout(
      ~showLineNumbers=Config.lineNumbers.get(config) != `Off,
      ~maxMinimapCharacters=Config.Minimap.maxColumn.get(config),
      ~isMinimapShown=Config.Minimap.enabled.get(config),
      editor,
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
    Selection.getRanges(Editor.selection(editor), buffer) |> Range.toHash;

  let diffMarkers =
    lineCount < Constants.diffMarkersMaxLineCount && showDiffMarkers
      ? EditorDiffMarkers.generate(buffer) : None;

  let smoothScroll = Config.Experimental.smoothScroll.get(config);

  let%hook (scrollY, _setScrollYImmediately) =
    Hooks.spring(
      ~target=Editor.scrollY(editor),
      ~restThreshold=10.,
      ~enabled=smoothScroll,
      scrollSpringOptions,
    );
  let%hook (scrollX, _setScrollXImmediately) =
    Hooks.spring(
      ~target=Editor.scrollX(editor),
      ~restThreshold=10.,
      ~enabled=smoothScroll,
      scrollSpringOptions,
    );

  let editor =
    editor
    |> Editor.scrollToPixelX(~pixelX=scrollX)
    |> Editor.scrollToPixelY(~pixelY=scrollY);

  let pixelHeight = Editor.getTotalHeightInPixels(editor);

  let (gutterWidth, gutterView) =
    <GutterView
      editor
      showScrollShadow={Config.Experimental.scrollShadow.get(config)}
      showLineNumbers={Config.lineNumbers.get(config)}
      height=pixelHeight
      colors
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
      isActiveSplit
      cursorPosition
      editor
      gutterWidth
      editorFont
      completions
      colors
      theme
      tokenTheme
    />
    {renderOverlays(~gutterWidth)}
    <View style=Styles.verticalScrollBar>
      <Scrollbar.Vertical
        dispatch
        editor
        cursorPosition
        width=Constants.scrollBarThickness
        height=pixelHeight
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
