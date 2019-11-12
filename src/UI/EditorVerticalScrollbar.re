/*
 * EditorVerticalScrollbar.re
 */

open Revery;
open Revery.UI;

open Oni_Core;
open Oni_Core.Types;
open Oni_Model;

let component = React.component("EditorVerticalScrollbar");

let absoluteStyle =
  Style.[position(`Absolute), top(0), bottom(0), left(0), right(0)];

let createElement =
    (
      ~state: State.t,
      ~editor: Editor.t,
      ~height as totalHeight,
      ~width as totalWidth,
      ~diagnostics: IntMap.t(list(Diagnostic.t)),
      ~metrics,
      ~children as _,
      (),
    ) =>
  component(hooks => {
    let scrollMetrics =
      Editor.getVerticalScrollbarMetrics(editor, totalHeight, metrics);

    let scrollThumbStyle =
      Style.[
        position(`Absolute),
        top(scrollMetrics.thumbOffset),
        left(0),
        width(totalWidth),
        height(scrollMetrics.thumbSize),
        backgroundColor(state.theme.scrollbarSliderActiveBackground),
      ];

    let totalPixel =
      Editor.getTotalSizeInPixels(editor, metrics) |> float_of_int;

    let bufferLineToScrollbarPixel = line => {
      let pixelY = float_of_int(line) *. state.editorFont.measuredHeight;
      int_of_float(
        pixelY
        /. (totalPixel +. float_of_int(metrics.pixelHeight))
        *. float_of_int(totalHeight),
      );
    };

    let cursorPosition =
      bufferLineToScrollbarPixel(
        Index.toZeroBasedInt(editor.cursorPosition.line),
      );
    let cursorSize = 2;

    let scrollCursorStyle =
      Style.[
        position(`Absolute),
        top(cursorPosition),
        left(0),
        width(totalWidth),
        height(cursorSize),
        backgroundColor(state.theme.foreground),
      ];

    let diagnosticLines =
      IntMap.bindings(diagnostics)
      |> List.map(binding => {
           let (key, _) = binding;
           key;
         });

    let diagnosticElements =
      List.map(
        line => {
          let diagTop = bufferLineToScrollbarPixel(line);

          let diagnosticStyle =
            Style.[
              position(`Absolute),
              top(diagTop),
              right(0),
              width(Constants.default.scrollBarThickness / 3),
              height(cursorSize),
              backgroundColor(Colors.red),
            ];
          <View style=diagnosticStyle />;
        },
        diagnosticLines,
      );

    let matchingPairStyle = t =>
      Style.[
        position(`Absolute),
        top(t - 3),
        left(4),
        right(4),
        height(8),
        backgroundColor(
          state.theme.editorOverviewRulerBracketMatchForeground,
        ),
      ];

    let matchingPairElements =
      switch (Selectors.getMatchingPairs(state, editor.bufferId)) {
      | None => []
      | Some(mp) =>
        let topLine =
          bufferLineToScrollbarPixel(Index.toInt0(mp.startPos.line));
        let botLine =
          bufferLineToScrollbarPixel(Index.toInt0(mp.endPos.line));
        [
          <View style={matchingPairStyle(topLine)} />,
          <View style={matchingPairStyle(botLine)} />,
        ];
      };

    let selectionStyle = (t, bot) => {
      Style.[
        position(`Absolute),
        top(t),
        left(0),
        right(0),
        height(bot - t),
        backgroundColor(
          Color.multiplyAlpha(0.5, state.theme.editorSelectionBackground),
        ),
      ];
    };
    let getSelectionElements = (selection: VisualRange.t) => {
      switch (selection.mode) {
      | Vim.Types.None => []
      | _ =>
        let topLine =
          bufferLineToScrollbarPixel(
            Index.toInt0(selection.range.startPosition.line),
          );
        let botLine =
          bufferLineToScrollbarPixel(
            Index.toInt0(selection.range.endPosition.line) + 1,
          );
        [<View style={selectionStyle(topLine, botLine)} />];
      };
    };

    let selectionElements = getSelectionElements(editor.selection);

    let searchMatches = t =>
      Style.[
        position(`Absolute),
        top(t - 3),
        left(4),
        right(4),
        height(8),
        backgroundColor(state.theme.editorFindMatchBackground),
      ];

    let searchHighlightToElement = ((line, _)) => {
      <View style={searchMatches(bufferLineToScrollbarPixel(line))} />;
    };

    let searchMatchElements =
      List.map(
        searchHighlightToElement,
        IntMap.bindings(
          Selectors.getSearchHighlights(state, editor.bufferId),
        ),
      );

    (
      hooks,
      <View style=absoluteStyle>
        <View style=scrollThumbStyle />
        <View style=scrollCursorStyle />
        <View style=absoluteStyle> ...selectionElements </View>
        <View style=absoluteStyle> ...diagnosticElements </View>
        <View style=absoluteStyle> ...matchingPairElements </View>
        <View style=absoluteStyle> ...searchMatchElements </View>
      </View>,
    );
  });
