/*
 * EditorVerticalScrollbar.re
 */

open Revery.UI;

open Oni_Core;

let component = React.component("EditorHorizontalScrollBar");

let absoluteStyle =
  Style.[position(`Absolute), top(0), bottom(0), left(0), right(0)];

let createElement =
    (
      ~state: State.t,
      ~width as totalWidth,
      ~children as _,
      (),
    ) =>
  component(hooks => {
    let scrollMetrics = Editor.getHorizontalScrollbarMetrics(state.editor, totalWidth);

    let scrollThumbStyle =
      Style.[
        position(`Absolute),
        bottom(0),
        left(scrollMetrics.thumbOffset),
        width(scrollMetrics.thumbSize),
        top(0),
        opacity(0.4),
        backgroundColor(state.theme.colors.scrollbarSliderActiveBackground),
      ];

    (
      hooks,
      <View style=absoluteStyle>
        <View style=scrollThumbStyle />
      </View>,
    );
  });
