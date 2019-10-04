/*
 * FlatList.re
 *
 * Virtualized list helper
 */

open Revery.UI;

type renderFunction = int => React.syntheticElement;

let component = React.component("FlatList");

let additionalRowsToRender = 1;

let render = (~menuHeight, ~rowHeight, ~count, ~scrollTop, ~renderItem) =>
  if (rowHeight <= 0) {
    []
  } else {
    let startRow = scrollTop / rowHeight;
    let startY = scrollTop mod rowHeight;
    let rowsToRender = min(menuHeight / rowHeight + additionalRowsToRender, count - startRow);
    let indicesToRender = List.init(rowsToRender, i => i + startRow);

    let itemView = (i) => {
      let rowY = (i - startRow) * rowHeight;

      <View
        style=
          Style.[
            position(`Absolute),
            top(rowY - startY),
            left(0),
            right(0),
            height(rowHeight),
          ]
        >
        {renderItem(i)}
      </View>
    };

  indicesToRender
    |> List.map(itemView)
    |> List.rev
  };

let createElement =
    (
      ~height as menuHeight,
      ~width as menuWidth,
      ~rowHeight: int,
      ~render as renderItem: renderFunction,
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
        } else if (offset + rowHeight > actualScrollTop + menuHeight) {
          // out of view below, so align with bottom edge
          setScrollTop(offset + rowHeight - menuHeight);
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
        |> min(rowHeight * count - menuHeight);

      setScrollTop(newScrollTop);
    };

    let items = render(
      ~menuHeight,
      ~rowHeight,
      ~count,
      ~scrollTop = actualScrollTop,
      ~renderItem
    );

    let style =
      Style.[
        position(`Relative),
        top(0),
        left(0),
        width(menuWidth),
        height(min(menuHeight, count * rowHeight)),
        overflow(`Hidden),
      ];

    (hooks, <View style onMouseWheel=scroll> ...items </View>);
  });
