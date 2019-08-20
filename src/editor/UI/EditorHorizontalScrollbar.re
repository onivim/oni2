/*
 * EditorVerticalScrollbar.re
 */

open Revery.UI;

open Oni_Model;

let component = React.component("EditorHorizontalScrollBar");

let absoluteStyle =
  Style.[position(`Absolute), top(0), bottom(0), left(0), right(0)];

let emptyElement = React.listToElement([]);

let createElement =
    (
      ~state: State.t,
      ~editor: Editor.t,
      ~width as totalWidth,
      ~metrics,
      ~children as _,
      (),
    ) =>
  component(hooks => {
    let scrollMetrics =
      Editor.getHorizontalScrollbarMetrics(editor, totalWidth, metrics);

    let scrollThumbStyle =
      Style.[
        position(`Absolute),
        bottom(0),
        left(scrollMetrics.thumbOffset),
        width(scrollMetrics.thumbSize),
        top(0),
        backgroundColor(state.theme.colors.scrollbarSliderActiveBackground),
      ];

    let elm =
      switch (scrollMetrics.visible) {
      | false => emptyElement
      | true =>
        <View style=absoluteStyle> <Opacity opacity=0.5><View style=scrollThumbStyle /></Opacity> </View>
      };

    (hooks, elm);
  });
