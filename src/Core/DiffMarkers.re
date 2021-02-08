module Log = (val Kernel.Log.withNamespace("DiffMarkers"));

[@deriving show({with_path: false})]
type t = array(marker)
and marker =
  | Modified
  | Added
  | DeletedBefore
  | DeletedAfter
  | Unmodified;

let get = (~line: EditorCoreTypes.LineNumber.t, markers) => {
  let lineIdx = line |> EditorCoreTypes.LineNumber.toZeroBased;

  if (lineIdx < 0 || lineIdx >= Array.length(markers)) {
    Log.warnf(m =>
      m("Tried to request out-of-range marker at index: %d", lineIdx)
    );
    Unmodified;
  } else {
    markers[lineIdx];
  };
};

let toArray = Fun.id;

let generate = (~originalLines, buffer: Buffer.t) => {
  // `adds` is an array of bools the length of the current lines array where `true` indicates the line is added
  // `deletes` is an array of bools the length of the originall lines array where `true` indicates the line has been deleted
  let (adds, deletes) = Diff.f(Buffer.getLines(buffer), originalLines);

  // shift is the offset between lines that should match up at the current index;
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
  if (markers == [||]) {
    [|DeletedBefore|];
  } else if (Array.length(deletes) - shift^ - 1 > Array.length(adds)) {
    markers[Array.length(markers) - 1] = (
      switch (markers[Array.length(markers) - 1]) {
      | Modified
      | Added => Modified

      | DeletedBefore
      | DeletedAfter
      | Unmodified => DeletedAfter
      }
    );
    markers;
  } else {
    markers;
  };
};
