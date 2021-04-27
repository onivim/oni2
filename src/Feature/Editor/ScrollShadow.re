/*
 * ScrollShadow.re
 *
 * Helpers for drawing a 'scroll shadow' effect, to visualize
 * scrollable area.
 */

module Constants = {
  let shadowSize = 12.;
};

let renderHorizontal = (~color, ~editor: Editor.t, ~width: float, ~context) => {
  let scrollX = Editor.scrollX(editor);
  let pixelHeight = Editor.visiblePixelHeight(editor);
  if (scrollX > 1.) {
    Draw.Shadow.render(
      ~color,
      ~direction=Right,
      ~context,
      ~x=0.,
      ~y=0.,
      ~width=Constants.shadowSize,
      ~height=float(pixelHeight),
    );
  };

  // Use 'floor' on total width - allow a fractional component, so we don't get a scroll shadow when word wrapped.
  if (scrollX +. width < floor(Editor.getTotalWidthInPixels(editor))) {
    let () =
      Draw.Shadow.render(
        ~color,
        ~direction=Left,
        ~context,
        ~x=width -. Constants.shadowSize -. 1.0,
        ~y=0.,
        ~width=Constants.shadowSize,
        ~height=float(pixelHeight),
      );
    ();
  };
};
let renderVertical = (~color, ~editor: Editor.t, ~width: float, ~context) => {
  let scrollY = Editor.scrollY(editor);
  let pixelHeight = Editor.visiblePixelHeight(editor);
  if (scrollY > 1.) {
    Draw.Shadow.render(
      ~color,
      ~direction=Down,
      ~context,
      ~x=0.,
      ~y=-2.,
      ~width,
      ~height=Constants.shadowSize,
    );
  };
  if (scrollY
      +. float(pixelHeight) < float(Editor.getTotalHeightInPixels(editor))) {
    let () =
      Draw.Shadow.render(
        ~color,
        ~direction=Up,
        ~context,
        ~x=0.,
        ~y=float(pixelHeight) -. Constants.shadowSize,
        ~width,
        ~height=Constants.shadowSize,
      );
    ();
  };
};
