/*
 * FlatList.re
 *
 * Virtualized list helper
 */

open Revery.UI;

type renderFunction = int => React.syntheticElement;

let component = React.component("FlatList");

let additionalRowsToRender = 1;

type glRenderFunction = (int, int) => unit;

let render = (~scrollY=0, ~rowHeight, ~height, ~count, ~render: glRenderFunction, ()) => {
    let rowsToRender = rowHeight > 0 ? height / rowHeight : 0;
    let startRowOffset = rowHeight > 0 ? scrollY / rowHeight : 0;
    let pixelOffset = scrollY mod rowHeight;

    let i = ref(max(startRowOffset - additionalRowsToRender, 0));

    let len = count;

    while (i^ < rowsToRender
           + additionalRowsToRender
           + startRowOffset
           && i^ < len) {
      let rowOffset = (i^ - startRowOffset) * rowHeight;
      /* let rowContainerStyle = */
      /*   Style.[ */
      /*     position(`Absolute), */
      /*     top(rowOffset - pixelOffset), */
      /*     left(0), */
      /*     right(0), */
      /*     height(rowHeight), */
      /*   ]; */

      let top = rowOffset - pixelOffset;

      let item = i^;
      render(item, top);

      incr(i);
    };
}

let createElement =
    (
      ~scrollY=0,
      ~height as height_,
      ~width as width_,
      ~rowHeight: int,
      ~render: renderFunction,
      ~count: int,
      ~children as _,
      (),
    ) =>
  component(hooks => {
    /* let (v, setV, hooks) = React.Hooks.state(0, hooks); */
    let rowsToRender = rowHeight > 0 ? height_ / rowHeight : 0;
    let startRowOffset = rowHeight > 0 ? scrollY / rowHeight : 0;
    let pixelOffset = scrollY mod rowHeight;

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

    items := List.rev(items^);

    let style =
      Style.[
        position(`Absolute),
        top(0),
        left(0),
        width(width_),
        height(height_),
      ];

    let scroll = (wheelEvent: NodeEvents.mouseWheelEventParams) => {
      GlobalContext.current().editorScroll(
        ~deltaY=int_of_float(wheelEvent.deltaY) * 25,
        (),
      );
    };

    (hooks, <View style onMouseWheel=scroll> ...items^ </View>);
  });
