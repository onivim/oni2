/*
 * FlatList.re
 *
 * Virtualized list helper
 */

open Revery;
open Revery.UI;
open Revery_UI_Components;

module Utility = Oni_Core.Utility;

type renderFunction = int => React.syntheticElement;

module Constants = {
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

let component = React.component("FlatList");

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

let createElement =
    (
      ~height as menuHeight,
      ~width,
      ~rowHeight: int,
      ~render as renderItem: renderFunction,
      ~count: int,
      ~selected: option(int),
      ~children as _,
      (),
    ) =>
  component(hooks => {
    let (actualScrollTop, setScrollTop, hooks) = Hooks.state(0, hooks);

    let selectedChanged = () => {
      switch (selected) {
      | Some(selectedIndex) =>
        let offset = selectedIndex * rowHeight;
        if (offset < actualScrollTop) {
          // out of view above, so align with top edge
          setScrollTop(offset);
        } else if (offset + rowHeight > actualScrollTop + menuHeight) {
          // out of view below, so align with bottom edge
          setScrollTop(
            offset + rowHeight - menuHeight,
          );
        };
      | None => ()
      };
      None;
    };

    let hooks = Hooks.effect(If((!=), selected), selectedChanged, hooks);

    let scroll = (wheelEvent: NodeEvents.mouseWheelEventParams) => {
      let newScrollTop =
        actualScrollTop
        + int_of_float(wheelEvent.deltaY)
        * (-25)
        |> max(0)
        |> min(rowHeight * count - menuHeight);

      setScrollTop(newScrollTop);
    };

    let scrollbar = {
      let maxHeight = count * rowHeight - menuHeight;
      let thumbHeight = menuHeight * menuHeight / max(1, count * rowHeight);
      let isVisible = maxHeight > 0;

      if (isVisible) {
        <View style=Styles.slider>
          <Slider
            onValueChanged={v => setScrollTop(int_of_float(v))}
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
      );
    (
      hooks,
      <View
        style={Styles.container(
          ~width,
          ~height=min(menuHeight, count * rowHeight),
        )}
        onMouseWheel=scroll>
        <View
          style={Styles.viewport(
            ~isScrollbarVisible=scrollbar == React.empty,
          )}>
          ...items
        </View>
        scrollbar
      </View>,
    );
  });
