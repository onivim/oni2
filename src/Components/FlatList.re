/*
 * FlatList.re
 *
 * Virtualized list helper
 */
open Oni_Core;
open Utility;

open Revery.UI;
open Revery_UI_Components;

module Colors = Feature_Theme.Colors;

module Constants = {
  let scrollWheelMultiplier = 25;
  let additionalRowsToRender = 1;
  let scrollBarThickness = 6;
};

module Styles = {
  open Style;

  let container = (~height) => [
    position(`Relative),
    top(0),
    left(0),
    switch (height) {
    | Some(height) => Style.height(height)
    | None => flexGrow(1)
    },
    overflow(`Hidden),
  ];

  let slider = [
    position(`Absolute),
    right(0),
    top(0),
    bottom(0),
    width(Constants.scrollBarThickness),
  ];

  let viewport = (~isScrollbarVisible) => [
    position(`Absolute),
    top(0),
    left(0),
    bottom(0),
    right(isScrollbarVisible ? 0 : Constants.scrollBarThickness),
  ];

  let item = (~offset, ~rowHeight) => [
    position(`Absolute),
    top(offset),
    left(0),
    right(0),
    height(rowHeight),
  ];
};

let render = (~viewportHeight, ~rowHeight, ~count, ~scrollTop, ~renderItem) =>
  if (rowHeight <= 0) {
    [];
  } else {
    let startRow = scrollTop / rowHeight;
    let startY = scrollTop mod rowHeight;
    let rowsToRender =
      viewportHeight
      / rowHeight
      + Constants.additionalRowsToRender
      |> IntEx.clamp(~lo=0, ~hi=count - startRow);
    let indicesToRender = List.init(rowsToRender, i => i + startRow);

    let itemView = i => {
      let rowY = (i - startRow) * rowHeight;
      let offset = rowY - startY;

      <View style={Styles.item(~offset, ~rowHeight)}>
        {renderItem(i)}
      </View>;
    };

    indicesToRender |> List.map(itemView) |> List.rev;
  };

type action =
  | FocusedChanged
  | SetScrollTop(int);

let%component make =
              (
                ~rowHeight: int,
                ~initialRowsToRender=20,
                ~children as renderItem: int => React.element(React.node),
                ~count: int,
                ~focused: option(int),
                ~theme: ColorTheme.Colors.t,
                ~ref as onRef=_ => (),
                (),
              ) => {
  let%hook (viewportHeight, setViewportHeight) =
    Hooks.state(rowHeight * initialRowsToRender);
  let contentHeight = count * rowHeight;

  let reducer = (action, actualScrollTop) =>
    switch (action) {
    | FocusedChanged =>
      let offset = Option.value(focused, ~default=0) * rowHeight;
      if (offset < actualScrollTop) {
        // out of view above, so align with top edge
        offset;
      } else if (offset + rowHeight > actualScrollTop + viewportHeight) {
        // out of view below, so align with bottom edge
        offset + rowHeight - viewportHeight;
      } else {
        actualScrollTop;
      };
    | SetScrollTop(scrollTop) => scrollTop
    };

  let%hook (actualScrollTop, dispatch) =
    Hooks.reducer(~initialState=0, reducer);

  // Make sure we're not scrolled past the items
  let actualScrollTop =
    actualScrollTop
    |> IntEx.clamp(~lo=0, ~hi=rowHeight * count - viewportHeight);

  let%hook () =
    Hooks.effect(
      If((!=), focused),
      () => {
        dispatch(FocusedChanged);
        None;
      },
    );

  let scroll = (wheelEvent: NodeEvents.mouseWheelEventParams) => {
    let delta =
      int_of_float(wheelEvent.deltaY) * (- Constants.scrollWheelMultiplier);

    dispatch(SetScrollTop(actualScrollTop + delta));
  };

  let scrollbar = {
    let maxHeight = count * rowHeight - viewportHeight;
    let thumbHeight =
      viewportHeight * viewportHeight / max(1, count * rowHeight);
    let isVisible = maxHeight > 0;

    if (isVisible) {
      <View style=Styles.slider>
        <Slider
          onValueChanged={v => dispatch(SetScrollTop(int_of_float(v)))}
          minimumValue=0.
          maximumValue={float_of_int(maxHeight)}
          sliderLength=viewportHeight
          thumbLength=thumbHeight
          value={float_of_int(actualScrollTop)}
          trackThickness=Constants.scrollBarThickness
          thumbThickness=Constants.scrollBarThickness
          minimumTrackColor=Revery.Colors.transparentBlack
          maximumTrackColor=Revery.Colors.transparentBlack
          thumbColor={Colors.ScrollbarSlider.background.from(theme)}
          vertical=true
        />
      </View>;
    } else {
      React.empty;
    };
  };

  let items =
    render(
      ~viewportHeight,
      ~rowHeight,
      ~count,
      ~scrollTop=actualScrollTop,
      ~renderItem,
    )
    |> React.listToElement;

  <View
    style=Style.[flexGrow(1)]
    onDimensionsChanged={({height, _}) => {setViewportHeight(_ => height)}}>
    <View
      style={Styles.container(
        // Set the height only to force it smaller, not bigger
        ~height=contentHeight < viewportHeight ? Some(contentHeight) : None,
      )}
      ref=onRef
      onMouseWheel=scroll>
      <View
        style={Styles.viewport(~isScrollbarVisible=scrollbar == React.empty)}>
        items
      </View>
      scrollbar
    </View>
  </View>;
};
