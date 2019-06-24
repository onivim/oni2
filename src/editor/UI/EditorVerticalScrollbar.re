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
      ~diagnostics: IntMap.t(list(Diagnostics.Diagnostic.t)),
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
        backgroundColor(state.theme.colors.scrollbarSliderActiveBackground),
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
        backgroundColor(state.theme.colors.foreground),
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
          state.theme.colors.editorOverviewRulerBracketMatchForeground,
        ),
      ];

    print_endline(
      "Checking matching pairs for buffer: " ++ string_of_int(editor.bufferId),
    );

    let matchingPairElements =
      switch (Selectors.getMatchingPairs(state, editor.bufferId)) {
      | None => []
      | Some(mp) =>
        let topLine =
          bufferLineToScrollbarPixel(Index.toInt0(mp.startPos.line));
        let botLine =
          bufferLineToScrollbarPixel(Index.toInt0(mp.endPos.line));
        print_endline("GOT MATCHING PAIRS");
        [
          <View style={matchingPairStyle(topLine)} />,
          <View style={matchingPairStyle(botLine)} />,
        ];
      };

    (
      hooks,
      <View style=absoluteStyle>
        <View style=scrollThumbStyle />
        <View style=scrollCursorStyle />
        <View style=absoluteStyle> ...diagnosticElements </View>
        <View style=absoluteStyle> ...matchingPairElements </View>
      </View>,
    );
  });
