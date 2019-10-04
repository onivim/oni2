/*
 * FlatList.re
 *
 * Virtualized list helper
 */

open Revery.UI;

type renderFunction = int => React.syntheticElement;

let component = React.component("FlatList");

let additionalRowsToRender = 1;

let createElement =
    (
      ~height as height_,
      ~width as width_,
      ~rowHeight: int,
      ~render: renderFunction,
      ~count: int,
      ~selected: option(int),
      ~children as _,
      (),
    ) =>
  component(hooks => {
    let (actualScrollTop, setScrollTop, hooks) =
      Hooks.state(0, hooks);
    
    let selectedChanged = () => {
      switch (selected) {
      | Some(selectedIndex) =>
        let offset = selectedIndex * rowHeight;
        if (offset < actualScrollTop) {
          // out of view above, so align with top edge
          setScrollTop(offset);
        } else if (offset + rowHeight > actualScrollTop + height_) {
          // out of view below, so align with bottom edge
          setScrollTop(offset + rowHeight - height_);
        }
      | None =>
        ()
      };
      None
    };

    let hooks =
      Hooks.effect(If((!=), selected), selectedChanged, hooks);

    let scroll = (wheelEvent: NodeEvents.mouseWheelEventParams) => {
      let newScrollTop =
        actualScrollTop + int_of_float(wheelEvent.deltaY) * -25
        |> max(0)
        |> min(rowHeight * count - height_);

      setScrollTop(newScrollTop);
    };

    let rowsToRender = rowHeight > 0 ? height_ / rowHeight : 0;
    let startRowOffset =
        rowHeight > 0 ? actualScrollTop / rowHeight : 0;

    let pixelOffset = actualScrollTop mod rowHeight;

    let i = ref(max(startRowOffset - additionalRowsToRender, 0));

    let items: ref(list(React.syntheticElement)) = ref([]);

    let len = count;

    while (i^ < rowsToRender
           + additionalRowsToRender
           + startRowOffset
           && i^ < len) {
      let rowOffset = (i^ - startRowOffset) * rowHeight;
      let rowContainerStyle =
        Style.[
          position(`Absolute),
          top(rowOffset - pixelOffset),
          left(0),
          right(0),
          height(rowHeight),
        ];

      let item = i^;
      let v = <View style=rowContainerStyle> {render(item)} </View>;

      items := List.append([v], items^);
      incr(i);
    };

    let height_ = min(height_, count * rowHeight);

    items := List.rev(items^);

    let style =
      Style.[
        position(`Relative),
        top(0),
        left(0),
        width(width_),
        height(height_),
        overflow(`Hidden),
      ];

    (hooks, <View style onMouseWheel=scroll> ...items^ </View>);
  });
