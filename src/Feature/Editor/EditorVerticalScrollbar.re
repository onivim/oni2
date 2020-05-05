/*
 * EditorVerticalScrollbar.re
 */

open EditorCoreTypes;
open Revery.UI;

open Oni_Core;

module BufferHighlights = Oni_Syntax.BufferHighlights;
module Diagnostic = Feature_LanguageSupport.Diagnostic;

let absoluteStyle =
  Style.[position(`Absolute), top(0), bottom(0), left(0), right(0)];

let make =
    (
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
  let scrollMetrics = Editor.getVerticalScrollbarMetrics(editor, totalHeight);

  let scrollThumbStyle =
    Style.[
      position(`Absolute),
      top(scrollMetrics.thumbOffset),
      left(0),
      width(totalWidth),
      height(scrollMetrics.thumbSize),
      backgroundColor(colors.scrollbarSliderBackground),
    ];

  let totalPixel = Editor.getTotalSizeInPixels(editor) |> float_of_int;

  let bufferLineToScrollbarPixel = line => {
    let pixelY = float_of_int(line) *. editorFont.measuredHeight;
    int_of_float(
      pixelY
      /. (totalPixel +. float_of_int(editor.pixelHeight))
      *. float_of_int(totalHeight),
    );
  };

  let cursorLine =
    bufferLineToScrollbarPixel(
      Index.toZeroBased(Location.(cursorPosition.line)),
    );
  let cursorSize = 2;

  let scrollCursorStyle =
    Style.[
      position(`Absolute),
      top(cursorLine),
      left(0),
      width(totalWidth),
      height(cursorSize),
      backgroundColor(colors.editorForeground),
    ];

  let diagnosticElements =
    IntMap.bindings(diagnostics)
    |> List.map(binding => {
         let (key, _) = binding;
         key;
       })
    |> List.map(line => {
         let diagTop = bufferLineToScrollbarPixel(line);

         let diagnosticStyle =
           Style.[
             position(`Absolute),
             top(diagTop),
             right(0),
             width(Constants.scrollBarThickness / 3),
             height(cursorSize),
             backgroundColor(colors.errorForeground),
           ];
         <View style=diagnosticStyle />;
       })
    |> React.listToElement;

  let matchingPairStyle = t =>
    Style.[
      position(`Absolute),
      top(t - 3),
      left(4),
      right(4),
      height(8),
      backgroundColor(colors.overviewRulerBracketMatchForeground),
    ];

  let matchingPairElements =
    BufferHighlights.getMatchingPair(editor.bufferId, bufferHighlights)
    |> Option.map(mp => {
         let (startPos, endPos) = mp;
         let topLine =
           bufferLineToScrollbarPixel(
             Index.toZeroBased(Location.(startPos.line)),
           );
         let botLine =
           bufferLineToScrollbarPixel(
             Index.toZeroBased(Location.(endPos.line)),
           );
         React.listToElement([
           <View style={matchingPairStyle(topLine)} />,
           <View style={matchingPairStyle(botLine)} />,
         ]);
       })
    |> Option.value(~default=React.empty);

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
        bufferLineToScrollbarPixel(
          Index.toZeroBased(selection.range.start.line),
        );
      let botLine =
        bufferLineToScrollbarPixel(
          Index.toZeroBased(selection.range.stop.line) + 1,
        );
      [<View style={selectionStyle(topLine, botLine)} />];
    };
  };

  let selectionElements =
    getSelectionElements(editor.selection) |> React.listToElement;

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
    <View style={searchMatches(bufferLineToScrollbarPixel(line))} />;
  };

  let searchMatchElements =
    BufferHighlights.getHighlights(
      ~bufferId=editor.bufferId,
      bufferHighlights,
    )
    |> List.map(searchHighlightToElement)
    |> React.listToElement;

  <View style=absoluteStyle>
    <View style=scrollThumbStyle />
    <View style=scrollCursorStyle />
    <View style=absoluteStyle> selectionElements </View>
    <View style=absoluteStyle> diagnosticElements </View>
    <View style=absoluteStyle> matchingPairElements </View>
    <View style=absoluteStyle> searchMatchElements </View>
  </View>;
};
