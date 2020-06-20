/*
 * ScrollShadow.re
 *
 * Helpers for drawing a 'scroll shadow' effect, to visualize
 * scrollable area.
 */

module Constants = {
  let shadowSize = 12.;
};

let renderHorizontal = (~editor: Editor.t, ~width: float, ~context) => {
  if (editor.scrollX > 1.) {
    Draw.Shadow.render(
      ~direction=Right,
      ~context,
      ~x=0.,
      ~y=0.,
      ~width=Constants.shadowSize,
      ~height=float(editor.pixelHeight),
    );
  };

  if (editor.scrollX +. width < float(Editor.getTotalWidthInPixels(editor))) {
    let () =
      Draw.Shadow.render(
        ~direction=Left,
        ~context,
        ~x=width -. Constants.shadowSize -. 1.0,
        ~y=0.,
        ~width=Constants.shadowSize,
        ~height=float(editor.pixelHeight),
      );
    ();
  };
};
let renderVertical = (~editor: Editor.t, ~width: float, ~context) => {
  if (editor.scrollY > 1.) {
    Draw.Shadow.render(
      ~direction=Down,
      ~context,
      ~x=0.,
      ~y=-2.,
      ~width,
      ~height=Constants.shadowSize,
    );
  };
  if (editor.scrollY
      +. float(editor.pixelHeight)
      < float(Editor.getTotalHeightInPixels(editor))) {
    let () =
      Draw.Shadow.render(
        ~direction=Up,
        ~context,
        ~x=0.,
        ~y=float(editor.pixelHeight) -. Constants.shadowSize,
        ~width,
        ~height=Constants.shadowSize,
      );
    ();
  };
};
