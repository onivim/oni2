open Revery;
open Revery.UI;
open Revery.UI.Components;

module Constants = {
  let scrollWheelMultiplier = 25;
  let scrollBarThickness = 10;
  let scrollTrackColor = Color.rgba(0., 0., 0., 0.4);
  let scrollThumbColor = Color.rgba(0.5, 0.5, 0.5, 0.4);
};

module Styles = {
  open Style;
  module Colors = Feature_Theme.Colors;

  let outer = (~x, ~y, ~theme) => [
    position(`Absolute),
    left(x),
    top(y),
    border(~width=1, ~color=Colors.EditorHoverWidget.border.from(theme)),
  ];

  let maxHeight = 200;
  let maxWidth = 500;

  let container = [
    position(`Relative),
    Style.maxWidth(maxWidth + Constants.scrollBarThickness),
    Style.maxHeight(maxHeight),
    overflow(`Scroll),
  ];

  let contents = (~theme, ~showScrollbar, ~scrollTop) => [
    backgroundColor(Colors.EditorHoverWidget.background.from(theme)),
    Style.maxWidth(maxWidth),
    top(scrollTop),
    paddingLeft(6),
    {
      showScrollbar
        ? paddingRight(6 + Constants.scrollBarThickness) : paddingRight(6);
    },
    paddingBottom(4),
    paddingTop(4),
  ];

  let scrollBar = (~theme) => [
    right(0),
    top(0),
    bottom(0),
    position(`Absolute),
    backgroundColor(Colors.EditorHoverWidget.background.from(theme)),
    width(Constants.scrollBarThickness),
  ];
};

type state = {
  scrollTop: int,
  maybeHeight: option(int),
}
and action =
  | SetScrollTop(int)
  | SetHeight(int);

let initialState = {scrollTop: 0, maybeHeight: None};

let reducer = (action, state) =>
  switch (action) {
  | SetScrollTop(scrollTop) => {...state, scrollTop}
  | SetHeight(height) => {...state, maybeHeight: Some(height)}
  };

let%component make = (~x, ~y, ~theme, ~children=React.empty, ()) => {
  let%hook (state, dispatch) = Hooks.reducer(~initialState, reducer);

  let showScrollbar =
    switch (state.maybeHeight) {
    | None => false
    | Some(height) => height >= Styles.maxHeight
    };

  let scrollbar = () =>
    switch (state.maybeHeight) {
    | None => React.empty
    | Some(height) =>
      let thumbLength = Styles.maxHeight * Styles.maxHeight / height;
      <View style={Styles.scrollBar(~theme)}>
        <Slider
          onValueChanged={v => dispatch(SetScrollTop(int_of_float(v)))}
          value={float(state.scrollTop)}
          minimumValue=0.
          maximumValue={float(Styles.maxHeight - height)}
          sliderLength=Styles.maxHeight
          thumbLength
          trackThickness=Constants.scrollBarThickness
          thumbThickness=Constants.scrollBarThickness
          minimumTrackColor=Constants.scrollTrackColor
          maximumTrackColor=Constants.scrollTrackColor
          thumbColor=Constants.scrollThumbColor
          vertical=true
        />
      </View>;
    };

  let scroll = (wheelEvent: NodeEvents.mouseWheelEventParams) =>
    switch (state.maybeHeight, showScrollbar) {
    | (Some(height), true) =>
      let delta =
        int_of_float(wheelEvent.deltaY) * Constants.scrollWheelMultiplier;
      dispatch(
        SetScrollTop(
          state.scrollTop
          + delta
          |> Oni_Core.Utility.IntEx.clamp(
               ~hi=0,
               ~lo=Styles.maxHeight - height,
             ),
        ),
      );

    | _ => ()
    };
  <View style={Styles.outer(~x, ~y, ~theme)}>
    <View style=Styles.container>
      <View
        style={Styles.contents(
          ~theme,
          ~showScrollbar,
          ~scrollTop=state.scrollTop,
        )}
        onMouseWheel=scroll
        onDimensionsChanged={({height, _}) => dispatch(SetHeight(height))}>
        ...children
      </View>
    </View>
    {showScrollbar ? <scrollbar /> : React.empty}
  </View>;
};
