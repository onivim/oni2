/*
 * EditorVerticalScrollbar.re
 */

open EditorCoreTypes;
open Revery;
open Revery.UI;

open Oni_Core;

module BufferHighlights = Oni_Syntax.BufferHighlights;
module Diagnostic = Feature_LanguageSupport.Diagnostic;
module Colors = Feature_Theme.Colors;

let absoluteStyle =
  Style.[position(`Absolute), top(0), bottom(0), left(0), right(0)];

let make =
    (
      ~editor: Editor.t,
      ~height as totalHeight,
      ~width as totalWidth,
      ~diagnostics: IntMap.t(list(Diagnostic.t)),
      ~metrics,
      ~theme,
      ~editorFont: Service_Font.font,
      ~bufferHighlights,
      (),
    ) => {
  let scrollMetrics =
    Editor.getVerticalScrollbarMetrics(editor, totalHeight, metrics);

  let scrollThumbStyle =
    Style.[
      position(`Absolute),
      top(scrollMetrics.thumbOffset),
      left(0),
      width(totalWidth),
      height(scrollMetrics.thumbSize),
      backgroundColor(Colors.ScrollbarSlider.background.from(theme)),
    ];

  let totalPixel =
    Editor.getTotalSizeInPixels(editor, metrics) |> float_of_int;

  let bufferLineToScrollbarPixel = line => {
    let pixelY = float_of_int(line) *. editorFont.measuredHeight;
    int_of_float(
      pixelY
      /. (totalPixel +. float_of_int(metrics.pixelHeight))
      *. float_of_int(totalHeight),
    );
  };

  let cursorPosition = Editor.getPrimaryCursor(editor);

  let cursorPosition =
    bufferLineToScrollbarPixel(Index.toZeroBased(cursorPosition.line));
  let cursorSize = 2;

  let scrollCursorStyle =
    Style.[
      position(`Absolute),
      top(cursorPosition),
      left(0),
      width(totalWidth),
      height(cursorSize),
      backgroundColor(Colors.foreground.from(theme)),
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
             backgroundColor(Revery.Colors.red),
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
      backgroundColor(
        Colors.EditorOverviewRuler.bracketMatchForeground.from(theme),
      ),
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
        Color.multiplyAlpha(
          0.5,
          Colors.Editor.selectionBackground.from(theme),
        ),
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
      backgroundColor(Colors.Editor.findMatchBackground.from(theme)),
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
