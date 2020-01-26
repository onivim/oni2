open Oni_Core;
open Utility;

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
       let (adds, deletes) = Diff.f(Buffer.getLines(buffer), originalLines);

       let shift = ref(0); // the offset between matching lines in deletes vs adds; ie. deletes[i + shift] == adds[i]
       let markers =
         Array.mapi(
           (i, added) => {
             let deleted = {
               let j = i + shift^;
               j >= Array.length(deletes) ? false : deletes[j];
             };

             switch (added, deleted) {
             | (true, true) => Modified

             | (true, false) =>
               decr(shift);
               Added;

             | (false, true) =>
               incr(shift);
               while (i
                      + shift^ < Array.length(deletes)
                      && deletes[i + shift^]) {
                 incr(shift);
               };
               DeletedBefore;

             | (false, false) => Unmodified
             };
           },
           adds,
         );

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

let renderMarker =
    (~x, ~y, ~rowHeight, ~width, ~transform, ~theme: Theme.t, marker) => {
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

  Shapes.drawRect(~transform, ~x, ~y, ~height, ~width, ~color, ());
};

let render =
    (
      ~scrollY,
      ~rowHeight,
      ~x,
      ~height,
      ~width,
      ~count,
      ~transform,
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
            ~transform,
            ~theme,
            markers[i],
          );
        },
    (),
  );
