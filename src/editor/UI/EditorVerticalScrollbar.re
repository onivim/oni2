/*
 * EditorVerticalScrollbar.re
 */

open Revery.Core;
open Revery.UI;

open Oni_Core;

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
    let scrollMetrics = Oni_Core.EditorView.getScrollbarMetrics(state.editorView, totalHeight, state.editorFont.measuredHeight);

    let scrollThumbStyle = Style.[
        position(`Absolute),   
        top(scrollMetrics.thumbOffset),
        left(0),
        width(totalWidth),
        height(scrollMetrics.thumbSize),
        backgroundColor(Colors.red),
    ];

    (
      hooks,
      <View style=absoluteStyle>
        <View style={scrollThumbStyle} />
      </View>,
    );
  });
