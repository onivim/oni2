open Oni_Core;
open DiffMarkers;

open Revery.Draw;

let markerPaint = Skia.Paint.make();
let renderMarker =
    (~x, ~y, ~rowHeight, ~width, ~canvasContext, ~colors: Colors.t, marker) => {
  let (y, height) =
    switch (marker) {
    | Modified
    | Added => (y, rowHeight)
    | DeletedBefore => (y -. width /. 2., width)
    | DeletedAfter => (y +. rowHeight -. width /. 2., width)
    | Unmodified => failwith("unreachable")
    };

  let color =
    switch (marker) {
    | Modified => colors.gutterModifiedBackground
    | Added => colors.gutterAddedBackground
    | DeletedBefore => colors.gutterDeletedBackground
    | DeletedAfter => colors.gutterDeletedBackground
    | Unmodified => failwith("unreachable")
    };

  let color = Revery.Color.toSkia(color);
  Skia.Paint.setColor(markerPaint, color);
  CanvasContext.drawRectLtwh(
    ~left=x,
    ~top=y,
    ~height,
    ~width,
    ~paint=markerPaint,
    canvasContext,
  );
};

let render =
    (
      ~context: Draw.context,
      ~rowHeight,
      ~x,
      ~width,
      ~canvasContext,
      ~colors,
      markers,
    ) =>
  Draw.renderImmediate(
    ~context,
    (i, y) => {
      let line = Editor.viewLineToBufferLine(i, context.editor);

      let marker = get(~line, markers);

      if (marker != Unmodified) {
        renderMarker(
          ~x,
          ~y,
          ~rowHeight,
          ~width,
          ~canvasContext,
          ~colors,
          marker,
        );
      };
    },
  );

let renderMinimap =
    (
      ~editor,
      ~scrollY,
      ~rowHeight,
      ~x,
      ~height,
      ~width,
      ~count,
      ~canvasContext,
      ~colors,
      markers,
    ) =>
  ImmediateList.render(
    ~scrollY,
    ~rowHeight,
    ~height,
    ~count,
    ~render=
      (i, y) => {
        let line = Editor.viewLineToBufferLine(i, editor);

        let marker = get(~line, markers);
        if (marker != Unmodified) {
          renderMarker(
            ~x,
            ~y,
            ~rowHeight,
            ~width,
            ~canvasContext,
            ~colors,
            marker,
          );
        };
      },
    (),
  );
