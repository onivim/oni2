/*
 * FlatList.re
 *
 * Virtualized list helper
 */

open Revery;
open Revery.UI;
open Revery_UI_Components;

module Utility = Oni_Core.Utility;

// TODO: Remove after 4.08 upgrade
module Option = {
  let map = f =>
    fun
    | Some(x) => Some(f(x))
    | None => None;

  let value = (~default) =>
    fun
    | Some(x) => x
    | None => default;

  let some = x => Some(x);
};

type renderFunction = int => React.element(React.node);

module Constants = {
  let scrollWheelMultiplier = 25;
  let additionalRowsToRender = 1;
  let scrollBarThickness = 6;
  let scrollTrackColor = Color.rgba(0.0, 0.0, 0.0, 0.4);
  let scrollThumbColor = Color.rgba(0.5, 0.5, 0.5, 0.4);
};

module Styles = {
  let container = (~width as w, ~height as h) =>
    Style.[
      position(`Relative),
      top(0),
      left(0),
      width(w),
      height(h),
      overflow(`Hidden),
    ];

  let slider =
    Style.[
      position(`Absolute),
      right(0),
      top(0),
      bottom(0),
      width(Constants.scrollBarThickness),
    ];

  let viewport = (~isScrollbarVisible) =>
    Style.[
      position(`Absolute),
      top(0),
      left(0),
      bottom(0),
      right(isScrollbarVisible ? 0 : Constants.scrollBarThickness),
    ];

  let item = (~offset, ~rowHeight) =>
    Style.[
      position(`Absolute),
      top(offset),
      left(0),
      right(0),
      height(rowHeight),
    ];
};

let render = (~menuHeight, ~rowHeight, ~count, ~scrollTop, ~renderItem) =>
  if (rowHeight <= 0) {
    [];
  } else {
    let startRow = scrollTop / rowHeight;
    let startY = scrollTop mod rowHeight;
    let rowsToRender =
      menuHeight
      / rowHeight
      + Constants.additionalRowsToRender
      |> Utility.clamp(~lo=0, ~hi=count - startRow);
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
                ~height as menuHeight,
                ~width,
                ~rowHeight: int,
                ~render as renderItem: renderFunction,
                ~count: int,
                ~focused: option(int),
                (),
              ) => {
  let reducer = (action, actualScrollTop) =>
    switch (action) {
    | FocusedChanged =>
      let offset = Option.value(focused, ~default=0) * rowHeight;
      if (offset < actualScrollTop) {
        // out of view above, so align with top edge
        offset;
      } else if (offset + rowHeight > actualScrollTop + menuHeight) {
        // out of view below, so align with bottom edge
        offset + rowHeight - menuHeight;
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
    |> Utility.clamp(~lo=0, ~hi=rowHeight * count - menuHeight);

  let%hook hooks =
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
    let maxHeight = count * rowHeight - menuHeight;
    let thumbHeight = menuHeight * menuHeight / max(1, count * rowHeight);
    let isVisible = maxHeight > 0;

    if (isVisible) {
      <View style=Styles.slider>
        <Slider
          onValueChanged={v => dispatch(SetScrollTop(int_of_float(v)))}
          minimumValue=0.
          maximumValue={float_of_int(maxHeight)}
          sliderLength=menuHeight
          thumbLength=thumbHeight
          value={float_of_int(actualScrollTop)}
          trackThickness=Constants.scrollBarThickness
          thumbThickness=Constants.scrollBarThickness
          minimumTrackColor=Constants.scrollTrackColor
          maximumTrackColor=Constants.scrollTrackColor
          thumbColor=Constants.scrollThumbColor
          vertical=true
        />
      </View>;
    } else {
      React.empty;
    };
  };

  let items =
    render(
      ~menuHeight,
      ~rowHeight,
      ~count,
      ~scrollTop=actualScrollTop,
      ~renderItem,
    )
    |> React.listToElement;

  <View
    style={Styles.container(
      ~width,
      ~height=min(menuHeight, count * rowHeight),
    )}
    onMouseWheel=scroll>
    <View
      style={Styles.viewport(~isScrollbarVisible=scrollbar == React.empty)}>
      items
    </View>
    scrollbar
  </View>;
};
