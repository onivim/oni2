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
      ~diagnostics: list(Diagnostics.Diagnostic.t),
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
      let pixelY =
        float_of_int(Index.toZeroBasedInt(line))
        *. state.editorFont.measuredHeight;
      int_of_float(
        pixelY
        /. (totalPixel +. float_of_int(metrics.pixelHeight))
        *. float_of_int(totalHeight),
      );
    };

    let cursorPosition =
      bufferLineToScrollbarPixel(editor.cursorPosition.line);
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

    let diagnosticElements =
      List.map(
        (d: Diagnostics.Diagnostic.t) => {
          let diagTop =
            bufferLineToScrollbarPixel(d.range.startPosition.line);

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
        diagnostics,
      );

    (
      hooks,
      <View style=absoluteStyle>
        <View style=scrollThumbStyle />
        <View style=scrollCursorStyle />
        <View style=absoluteStyle> ...diagnosticElements </View>
      </View>,
    );
  });
