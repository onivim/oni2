/*
 * EditorVerticalScrollbar.re
 */

open Revery.UI;

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

    let cursorPixelY =
      float_of_int(Index.toZeroBasedInt(editor.cursorPosition.line))
      *. state.editorFont.measuredHeight;
    let totalPixel =
      Editor.getTotalSizeInPixels(editor, metrics) |> float_of_int;

    let cursorPosition =
      int_of_float(
        cursorPixelY
        /. (totalPixel +. float_of_int(metrics.pixelHeight))
        *. float_of_int(totalHeight),
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

    (
      hooks,
      <View style=absoluteStyle>
        <View style=scrollThumbStyle />
        <View style=scrollCursorStyle />
      </View>,
    );
  });
