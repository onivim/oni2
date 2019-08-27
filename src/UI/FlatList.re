/*
 * FlatList.re
 *
 * Virtualized list helper
 */

open Revery.UI;

type renderFunction = int => React.syntheticElement;

let component = React.component("FlatList");

let additionalRowsToRender = 1;

type glRenderFunction = (int, float) => unit;

let render =
    (~scrollY=0., ~rowHeight, ~height, ~count, ~render: glRenderFunction, ()) => {
  let rowsToRender = rowHeight > 0. ? int_of_float(height /. rowHeight) : 0;
  let startRowOffset =
    rowHeight > 0. ? int_of_float(scrollY /. rowHeight) : 0;
  let pixelOffset = mod_float(scrollY, rowHeight);

  let i = ref(max(startRowOffset - additionalRowsToRender, 0));

  let len = count;

  while (i^ < rowsToRender
         + additionalRowsToRender
         + startRowOffset
         && i^ < len) {
    let item = i^;
    let rowOffset = float_of_int(item - startRowOffset) *. rowHeight;

    let top = rowOffset -. pixelOffset;

    render(item, top);

    incr(i);
  };
};

let createElement =
    (
      ~scrollY=0.,
      ~height as height_,
      ~width as width_,
      ~rowHeight: int,
      ~render: renderFunction,
      ~rowsToRender: int,
      ~count: int,
      ~children as _,
      ~onScroll,
      (),
    ) =>
  component(hooks => {
    let startRowOffset =
      rowHeight > 0 ? int_of_float(scrollY) / rowHeight : 0;
    let i = ref(0);
    let items: ref(list(React.syntheticElement)) = ref([]);

    while (i^ < rowsToRender && count > 0) {
      let rowContainerStyle = Style.[left(0), right(0), height(rowHeight)];

      let item = i^;
      let v =
        <View style=rowContainerStyle>
          {render(item + startRowOffset)}
        </View>;

      items := List.append([v], items^);
      incr(i);
    };

    let height_ = min(height_, rowsToRender * rowHeight);

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

    (hooks, <View style onMouseWheel=onScroll> ...items^ </View>);
  });
