/*
 * EditorVerticalScrollbar.re
 */

open Revery.UI;

open Oni_Model;

let absoluteStyle =
  Style.[position(`Absolute), top(0), bottom(0), left(0), right(0)];

let make =
    (~state: State.t, ~editor: Editor.t, ~width as totalWidth, ~metrics, ()) => {
  let scrollMetrics =
    Editor.getHorizontalScrollbarMetrics(editor, totalWidth, metrics);

  let scrollThumbStyle =
    Style.[
      position(`Absolute),
      bottom(0),
      left(scrollMetrics.thumbOffset),
      width(scrollMetrics.thumbSize),
      top(0),
      backgroundColor(state.theme.scrollbarSliderActiveBackground),
    ];

  switch (scrollMetrics.visible) {
  | false => React.empty
  | true =>
    <View style=absoluteStyle>
      <Opacity opacity=0.5> <View style=scrollThumbStyle /> </Opacity>
    </View>
  };
};
