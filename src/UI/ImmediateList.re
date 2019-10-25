type glRenderFunction = (int, float) => unit;

let additionalRowsToRender = 1;

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
