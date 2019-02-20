/*
 * FlatList.re
 *
 * Virtualized list helper
 */

/* open Oni_Core; */
open Revery.UI;

type renderFunction('a) = 'a => React.syntheticElement;

let component = React.component("FlatList");

let additionalRowsToRender = 1;

let createElement =
    (
      ~height as height_,
      ~width as width_,
      ~rowHeight: int,
      ~render: renderFunction('a),
      ~data: array('a),
      ~children as _,
      (),
    ) =>
  component(hooks => {
    let (v, setV, hooks) = React.Hooks.state(0, hooks);
    let scrollY = v;
    let rowsToRender = rowHeight > 0 ? height_ / rowHeight : 0;
    let startRowOffset = rowHeight > 0 ? scrollY / rowHeight : 0;
    let pixelOffset = scrollY mod rowHeight;

    let i = ref(max(startRowOffset - additionalRowsToRender, 0));

    let items: ref(list(React.syntheticElement)) = ref([]);

    let len = Array.length(data);

    while (i^ < rowsToRender + additionalRowsToRender && i^ < len) {
      let rowOffset = (i^ - startRowOffset) * rowHeight;
      let rowContainerStyle = Style.[
        position(`Absolute),
            top(rowOffset - pixelOffset),
            left(0),
            right(0),
            height(rowHeight),
      ];

      let item = data[i^];
      let v = <View style={rowContainerStyle}>{render(item)}</View>;

      items := List.append([v], items^);
      i := i^ + 1;
    };

    items := List.rev(items^);

    let style =
      Style.[
        position(`Absolute),
        overflow(LayoutTypes.Hidden),
        top(0),
        left(0),
        width(width_),
        height(height_),
      ];

    let scroll = (wheelEvent: NodeEvents.mouseWheelEventParams) => {
        setV(max(v - (int_of_float(wheelEvent.deltaY) * 25), 0));
    };

    (hooks, <View style onMouseWheel={scroll}> ...items^ </View>);
  });
