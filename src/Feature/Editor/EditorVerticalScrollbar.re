/*
 * EditorVerticalScrollbar.re
 */

open EditorCoreTypes;
open Revery.UI;

open Oni_Core;

module BufferHighlights = Oni_Syntax.BufferHighlights;
module Diagnostic = Feature_LanguageSupport.Diagnostic;

module Styles = {
  open Style;

  let absolute = [
    position(`Absolute),
    top(0),
    bottom(0),
    left(0),
    right(0),
  ];

  let background = color => [backgroundColor(color), ...absolute];

  let container = (~opacity, ~colors: Colors.t) => {
    let color =
      colors.scrollbarSliderBackground |> Revery.Color.multiplyAlpha(opacity);

    [
      backgroundColor(color),
      cursor(Revery.MouseCursors.pointer),
      ...absolute,
    ];
  };

  let verticalThumb =
      (~width, ~scrollMetrics: Editor.scrollbarMetrics, ~color) => [
    position(`Absolute),
    top(scrollMetrics.thumbOffset),
    left(0),
    Style.width(width),
    height(scrollMetrics.thumbSize),
    backgroundColor(color),
  ];

  let cursor = (~cursorLine, ~totalWidth, ~colors: Colors.t) => {
    [
      position(`Absolute),
      top(cursorLine),
      left(0),
      width(totalWidth),
      height(Constants.scrollBarCursorSize),
      backgroundColor(colors.editorForeground),
    ];
  };
};

let diagnosticMarkers =
    (~diagnostics, ~totalHeight, ~editor, ~colors: Colors.t, ()) => {
  IntMap.bindings(diagnostics)
  |> List.map(binding => {
       let (key, _) = binding;
       key;
     })
  |> List.map(line => {
       let diagTop =
         Editor.projectLine(~line, ~pixelHeight=totalHeight, editor)
         |> int_of_float;

       let diagnosticStyle =
         Style.[
           position(`Absolute),
           top(diagTop),
           right(0),
           width(Constants.scrollBarThickness / 3),
           height(Constants.scrollBarCursorSize),
           backgroundColor(colors.errorForeground),
         ];
       <View style=diagnosticStyle />;
     })
  |> React.listToElement;
};

let matchingPairMarkers =
    (~bufferHighlights, ~totalHeight, ~editor, ~colors: Colors.t, ()) => {
  let matchingPairStyle = t =>
    Style.[
      position(`Absolute),
      top(t - 3),
      left(4),
      right(4),
      height(8),
      backgroundColor(colors.overviewRulerBracketMatchForeground),
    ];

  BufferHighlights.getMatchingPair(
    Editor.getBufferId(editor),
    bufferHighlights,
  )
  |> Option.map(mp => {
       open Location;
       let (startPos, endPos) = mp;

       let topLine =
         Editor.projectLine(
           ~line=Index.toZeroBased(startPos.line),
           ~pixelHeight=totalHeight,
           editor,
         )
         |> int_of_float;

       let botLine =
         Editor.projectLine(
           ~line=Index.toZeroBased(endPos.line),
           ~pixelHeight=totalHeight,
           editor,
         )
         |> int_of_float;

       React.listToElement([
         <View style={matchingPairStyle(topLine)} />,
         <View style={matchingPairStyle(botLine)} />,
       ]);
     })
  |> Option.value(~default=React.empty);
};

let searchMarkers =
    (~bufferHighlights, ~totalHeight, ~editor, ~colors: Colors.t, ()) => {
  let searchMatches = t =>
    Style.[
      position(`Absolute),
      top(t - 3),
      left(4),
      right(4),
      height(8),
      backgroundColor(colors.findMatchBackground),
    ];

  let searchHighlightToElement = line => {
    let line = Index.toZeroBased(line);
    let position =
      Editor.projectLine(~line, ~pixelHeight=totalHeight, editor)
      |> int_of_float;
    <View style={searchMatches(position)} />;
  };

  BufferHighlights.getHighlights(
    ~bufferId=Editor.getBufferId(editor),
    bufferHighlights,
  )
  |> List.map(searchHighlightToElement)
  |> React.listToElement;
};

let selectionMarkers = (~totalHeight, ~editor, ~colors: Colors.t, ()) => {
  let selectionStyle = (t, bot) => {
    Style.[
      position(`Absolute),
      top(t),
      left(0),
      right(0),
      height(bot - t),
      backgroundColor(
        Revery.Color.multiplyAlpha(0.5, colors.selectionBackground),
      ),
    ];
  };
  let getSelectionElements = (selection: VisualRange.t) => {
    switch (selection.mode) {
    | Vim.Types.None => []
    | _ =>
      let topLine =
        Editor.projectLine(
          ~line=Index.toZeroBased(selection.range.start.line),
          ~pixelHeight=totalHeight,
          editor,
        )
        |> int_of_float;
      let botLine =
        Editor.projectLine(
          ~line=Index.toZeroBased(selection.range.stop.line) + 1,
          ~pixelHeight=totalHeight,
          editor,
        )
        |> int_of_float;
      [<View style={selectionStyle(topLine, botLine)} />];
    };
  };

  getSelectionElements(editor.selection) |> React.listToElement;
};

let%component make =
              (
                ~dispatch: Msg.t => unit,
                ~editor: Editor.t,
                ~cursorPosition: Location.t,
                ~height as totalHeight,
                ~width as totalWidth,
                ~diagnostics: IntMap.t(list(Diagnostic.t)),
                ~colors: Colors.t,
                ~editorFont: Service_Font.font,
                ~bufferHighlights,
                (),
              ) => {
  let%hook (opacity, setOpacity) = Hooks.state(0.8);

  let%hook (captureMouse, captureState) =
    Hooks.mouseCapture(
      ~onMouseMove=
        (origin, evt: NodeEvents.mouseMoveEventParams) => {
          prerr_endline("evt: " ++ string_of_float(evt.mouseY));
          Some(origin);
        },
      ~onMouseUp=
        (origin, _evt) => {
          dispatch(Msg.VerticalScrollbarMouseRelease);
          prerr_endline("done!");
          None;
        },
      (),
    );

  let%hook (maybeBbox, setBbox) = Hooks.state(None);

  let scrollMetrics = Editor.getVerticalScrollbarMetrics(editor, totalHeight);

  //  let projectLine = line => {
  //    Editor.projectLine(~line, ~pixelHeight=totalHeight, editor);
  //  };

  let cursorLine =
    Editor.projectLine(
      ~line=Index.toZeroBased(Location.(cursorPosition.line)),
      ~pixelHeight=totalHeight,
      editor,
    )
    |> int_of_float;

  let onMouseOver = _ => {
    prerr_endline("over");
    setOpacity(_ => 1.0);
  };

  let onMouseLeave = _ => {
    prerr_endline("leave");
    setOpacity(_ => 0.8);
  };

  let onMouseDown =
      ({mouseY, _}: Revery.UI.NodeEvents.mouseButtonEventParams) => {
    prerr_endline("mouse down: " ++ string_of_float(mouseY));
    captureMouse(1);
  };

  <View
    style={Styles.container(~opacity, ~colors)}
    onMouseDown
    onMouseOver
    onMouseLeave
    onBoundingBoxChanged={bbox => setBbox(_ => Some(bbox))}>
    <Opacity opacity>
      <View
        style={Styles.verticalThumb(
          ~width=totalWidth,
          ~scrollMetrics,
          ~color=colors.scrollbarSliderBackground,
        )}
      />
      <View style={Styles.cursor(~cursorLine, ~totalWidth, ~colors)} />
      <View style=Styles.absolute>
        <selectionMarkers totalHeight editor colors />
        <diagnosticMarkers totalHeight editor diagnostics colors />
        <matchingPairMarkers bufferHighlights editor totalHeight colors />
        <searchMarkers bufferHighlights editor totalHeight colors />
      </View>
    </Opacity>
  </View>;
};
