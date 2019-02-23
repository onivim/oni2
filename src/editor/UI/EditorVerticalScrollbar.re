/*
 * EditorVerticalScrollbar.re
 */

open Revery.UI;

open Oni_Core;
open Oni_Core.Types;

let component = React.component("EditorVerticalScrollbar");

let absoluteStyle =
  Style.[position(`Absolute), top(0), bottom(0), left(0), right(0)];

let createElement =
    (
      ~state: State.t,
      ~height as totalHeight,
      ~width as totalWidth,
      ~children as _,
      (),
    ) =>
  component(hooks => {
    let scrollMetrics =
      Oni_Core.EditorView.getScrollbarMetrics(
        state.editorView,
        totalHeight,
        state.editorFont.measuredHeight,
      );

    let scrollThumbStyle =
      Style.[
        position(`Absolute),
        top(scrollMetrics.thumbOffset),
        left(0),
        width(totalWidth),
        height(scrollMetrics.thumbSize),
        backgroundColor(state.theme.scrollbarSliderActiveBackground),
      ];

    let cursorPixelY =
      Index.toZeroBasedInt(state.editorView.cursorPosition.line)
      * state.editorFont.measuredHeight
      |> float_of_int;
    let totalPixel =
      EditorView.getTotalSizeInPixels(
        state.editorView,
        state.editorFont.measuredHeight,
      )
      |> float_of_int;

    let cursorPosition =
      int_of_float(
        cursorPixelY
        /. (totalPixel +. float_of_int(state.editorView.size.pixelHeight))
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
        backgroundColor(state.theme.foreground),
      ];

    (
      hooks,
      <View style=absoluteStyle>
        <View style=scrollThumbStyle />
        <View style=scrollCursorStyle />
      </View>,
    );
  });
