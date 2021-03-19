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
module Diagnostics = Feature_Diagnostics;
module Diagnostic = Feature_Diagnostics.Diagnostic;

module Constants = {
  include Constants;
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

  let inactiveCover = (~colors: Colors.t, ~opacity) => [
    backgroundColor(colors.editorBackground),
    Style.opacity(opacity),
    pointerEvents(`Ignore),
    position(`Absolute),
    top(0),
    left(0),
    right(0),
    bottom(0),
  ];

  let verticalScrollBar = (~thickness) => [
    position(`Absolute),
    top(0),
    right(0),
    width(thickness),
    bottom(0),
  ];

  let horizontalScrollBar = (~thickness, gutterOffset, width) => [
    position(`Absolute),
    bottom(0),
    left(gutterOffset),
    Style.width(width),
    height(thickness),
  ];
};

let minimap =
    (
      ~vim,
      ~cursorPosition: CharacterPosition.t,
      ~colors,
      ~config,
      ~dispatch,
      ~matchingPairs,
      ~maybeYankHighlights,
      ~bufferSyntaxHighlights,
      ~selectionRanges,
      ~showMinimapSlider,
      ~editor,
      ~diffMarkers,
      ~diagnosticsMap,
      ~minimapWidthInPixels,
      ~languageSupport,
      (),
    ) => {
  let minimapPixelWidth = minimapWidthInPixels + Constants.minimapPadding * 2;
  let style =
    Style.[
      position(`Absolute),
      top(0),
      right(Editor.verticalScrollbarThickness(editor)),
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
      config
      cursorPosition
      dispatch
      width=minimapPixelWidth
      height=pixelHeight
      maybeYankHighlights
      count
      diagnostics=diagnosticsMap
      getTokensForLine={getTokensForLine(
        ~editor,
        ~vim,
        ~cursorLine=
          EditorCoreTypes.LineNumber.toZeroBased(cursorPosition.line),
        ~colors,
        ~matchingPairs,
        ~bufferSyntaxHighlights,
        ~scrollX=0.,
      )}
      selection=selectionRanges
      showSlider=showMinimapSlider
      colors
      vim
      diffMarkers
      languageSupport
    />
  </View>;
};

// VIEW

let%component make =
              (
                ~dispatch,
                ~languageConfiguration,
                ~languageInfo,
                ~grammarRepository,
                ~showDiffMarkers=true,
                ~backgroundColor: option(Revery.Color.t)=?,
                ~foregroundColor: option(Revery.Color.t)=?,
                ~buffer,
                ~onEditorSizeChanged,
                ~isActiveSplit: bool,
                ~editor: Editor.t,
                ~uiFont: Oni_Core.UiFont.t,
                ~theme,
                ~vim,
                ~bufferSyntaxHighlights,
                ~diagnostics,
                ~tokenTheme,
                ~languageSupport,
                ~buffers: Feature_Buffers.model,
                ~snippets: Feature_Snippets.model,
                ~windowIsFocused,
                ~perFileTypeConfig: Oni_Core.Config.fileTypeResolver,
                ~renderOverlays,
                (),
              ) => {
  let colors = Colors.precompute(theme);

  let mode = Editor.mode(editor);

  let%hook lastDimensions = Hooks.ref(None);

  let editorId = Editor.getId(editor);

  let fileType =
    buffer |> Oni_Core.Buffer.getFileType |> Oni_Core.Buffer.FileType.toString;
  let config = perFileTypeConfig(~fileType);

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

  let showYankHighlightAnimation = Config.yankHighlightEnabled.get(config);
  let maybeYankHighlights =
    showYankHighlightAnimation ? editor |> Editor.yankHighlight : None;

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

  let cursorPosition = Editor.getPrimaryCursor(editor);

  let layout = Editor.getLayout(editor);

  let matchingPairCheckPosition =
    Vim.Mode.isInsert(mode)
      ? CharacterPosition.{
          line: cursorPosition.line,
          character: CharacterIndex.(cursorPosition.character - 1),
        }
      : cursorPosition;

  let matchingPairs =
    !Config.matchBrackets.get(config)
      ? None
      : Editor.getNearestMatchingPair(
          ~characterPosition=matchingPairCheckPosition,
          ~pairs=LanguageConfiguration.brackets(languageConfiguration),
          editor,
        );

  let diagnosticsMap =
    Feature_Diagnostics.getDiagnosticsMap(diagnostics, buffer);
  let selectionRanges =
    editor
    |> Editor.selections
    |> List.map(selection => Selection.getRanges(selection, buffer))
    |> List.flatten
    |> ByteRange.toHash;

  let diffMarkers =
    // TODO: Port this
    //lineCount < Constants.diffMarkersMaxLineCount && showDiffMarkers
    showDiffMarkers
      ? Feature_Buffers.getOriginalDiff(
          ~bufferId=Oni_Core.Buffer.getId(buffer),
          buffers,
        )
      : None;
  //) : None;

  let pixelHeight = Editor.getTotalHeightInPixels(editor);

  let (gutterWidth, gutterView) =
    <GutterView
      editor
      showScrollShadow={Config.scrollShadow.get(config)}
      showLineNumbers={Editor.lineNumbers(editor)}
      height=pixelHeight
      colors
      count=lineCount
      editorFont
      cursorLine={EditorCoreTypes.LineNumber.toZeroBased(cursorPosition.line)}
      diffMarkers
    />;

  let hoverPopup = {
    let maybeHover =
      Feature_LanguageSupport.Hover.Popup.make(
        ~theme,
        ~uiFont,
        ~editorFont,
        ~model=languageSupport,
        ~diagnostics,
        ~tokenTheme,
        ~grammars=grammarRepository,
        ~buffer,
        ~editorId=Some(editorId),
        ~languageInfo,
      );

    maybeHover
    |> Option.map(((position: CharacterPosition.t, sections)) => {
         let ({x: pixelX, y: pixelY}: PixelPosition.t, _) =
           Editor.bufferCharacterPositionToPixel(~position, editor);
         let popupX = pixelX +. gutterWidth |> int_of_float;
         let popupTopY = pixelY |> int_of_float;
         let popupBottomY =
           pixelY +. Editor.lineHeightInPixels(editor) |> int_of_float;

         let popupAvailableWidth = layout.bufferWidthInPixels |> int_of_float;
         let popupAvailableHeight = pixelHeight;

         <Oni_Components.Popup
           x=popupX
           topY=popupTopY
           bottomY=popupBottomY
           availableWidth=popupAvailableWidth
           availableHeight=popupAvailableHeight
           sections
           theme
         />;
       })
    |> Option.value(~default=React.empty);
  };

  let coverAmount =
    1.0
    -. Feature_Configuration.GlobalConfiguration.inactiveWindowOpacity.get(
         config,
       );
  let opacityCover =
    isActiveSplit
      ? React.empty
      : <View style={Styles.inactiveCover(~colors, ~opacity=coverAmount)} />;

  let verticalScrollbarThickness = Editor.verticalScrollbarThickness(editor);
  let horizontalScrollbarThickness =
    Editor.horizontalScrollbarThickness(editor);

  <View style={Styles.container(~colors)} onDimensionsChanged>
    gutterView
    <SurfaceView
      buffer
      editor
      colors
      dispatch
      cursorPosition
      editorFont
      diagnosticsMap
      selectionRanges
      matchingPairs
      maybeYankHighlights
      vim
      languageSupport
      languageConfiguration
      bufferSyntaxHighlights
      mode
      snippets
      isActiveSplit
      gutterWidth
      bufferPixelWidth={int_of_float(layout.bufferWidthInPixels)}
      windowIsFocused
      config
      uiFont
      theme
    />
    {Editor.isMinimapEnabled(editor)
       ? <minimap
           editor
           diagnosticsMap
           vim
           cursorPosition
           colors
           config
           dispatch
           matchingPairs
           maybeYankHighlights
           bufferSyntaxHighlights
           selectionRanges
           showMinimapSlider={Config.Minimap.showSlider.get(config)}
           diffMarkers
           minimapWidthInPixels={layout.minimapWidthInPixels}
           languageSupport
         />
       : React.empty}
    <OverlaysView
      isActiveSplit
      cursorPosition
      editor
      gutterWidth
      editorFont
      languageSupport
      theme
      tokenTheme
      uiFont
      grammars=grammarRepository
      buffer
      languageInfo
    />
    {renderOverlays(~gutterWidth)}
    hoverPopup
    <View
      style={Styles.verticalScrollBar(~thickness=verticalScrollbarThickness)}>
      <Scrollbar.Vertical
        dispatch
        editor
        matchingPair=matchingPairs
        cursorPosition
        width=verticalScrollbarThickness
        height=pixelHeight
        diagnostics=diagnosticsMap
        colors
        vim
        languageSupport
      />
    </View>
    <View
      style={Styles.horizontalScrollBar(
        ~thickness=horizontalScrollbarThickness,
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
    opacityCover
  </View>;
};
