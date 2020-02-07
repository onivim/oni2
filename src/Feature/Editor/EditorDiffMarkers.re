open Oni_Core;

open Revery.Draw;

[@deriving show({with_path: false})]
type t = array(marker)

and marker =
  | Modified
  | Added
  | DeletedBefore
  | DeletedAfter
  | Unmodified;

let generate = buffer =>
  buffer
  |> Buffer.getOriginalLines
  |> Option.map(originalLines => {
       // `adds` is an array of bools the length of the current lines array where `true` indicates the line is added
       // `deletes` is an array of bools the length of the originall lines array where `true` indicates the line has been deleted
       let (adds, deletes) = Diff.f(Buffer.getLines(buffer), originalLines);

       // shift is he offset between lines that should match up at the current index;
       // ie. `deletes[i + shift] == adds[i]`
       let shift = ref(0);

       let isDeleted = i => {
         i < Array.length(deletes) && deletes[i];
       };

       // create a new marker array by mapping over `adds` while also taking the
       // corresponding flag in `deletes` into account
       let markers =
         Array.mapi(
           (i, isAdded) =>
             switch (isAdded, isDeleted(i + shift^)) {
             | (true, true) => Modified

             | (true, false) =>
               decr(shift);
               Added;

             | (false, true) =>
               incr(shift);
               // skip over subsequent deletes to line up `shift` with the next non-deleted line
               // the skipped over deletes will be represented by the first
               while (isDeleted(i + shift^)) {
                 incr(shift);
               };
               DeletedBefore;

             | (false, false) => Unmodified
             },
           adds,
         );

       // if there are deleted lines past the end of the current document, mark the last lines as having deletes afterwards, or being modified
       if (Array.length(deletes) - shift^ - 1 > Array.length(adds)) {
         markers[Array.length(markers) - 1] = (
           switch (markers[Array.length(markers) - 1]) {
           | Modified
           | Added => Modified

           | DeletedBefore
           | DeletedAfter
           | Unmodified => DeletedAfter
           }
         );
       };

       markers;
     });

let markerPaint = Skia.Paint.make();
let renderMarker =
    (~x, ~y, ~rowHeight, ~width, ~canvasContext, ~theme: Theme.t, marker) => {
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
    | Modified => theme.editorGutterModifiedBackground
    | Added => theme.editorGutterAddedBackground
    | DeletedBefore => theme.editorGutterDeletedBackground
    | DeletedAfter => theme.editorGutterDeletedBackground
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
      ~scrollY,
      ~rowHeight,
      ~x,
      ~height,
      ~width,
      ~count,
      ~canvasContext,
      ~theme,
      markers,
    ) =>
  ImmediateList.render(
    ~scrollY,
    ~rowHeight,
    ~height,
    ~count,
    ~render=
      (i, y) =>
        if (markers[i] != Unmodified) {
          renderMarker(
            ~x,
            ~y,
            ~rowHeight,
            ~width,
            ~canvasContext,
            ~theme,
            markers[i],
          );
        },
    (),
  );
