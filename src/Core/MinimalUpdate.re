open EditorCoreTypes;

[@deriving show]
type update =
  | Added({
      beforeLine: LineNumber.t,
      lines: array(string),
    })
  | Deleted({
      startLine: LineNumber.t,
      stopLine: LineNumber.t,
    })
  | Modified({
      line: LineNumber.t,
      original: string,
      updated: string,
    });

let shift = (startLine: LineNumber.t, update) => {
  let idx = LineNumber.toZeroBased(startLine);
  LineNumber.(
    switch (update) {
    | Added({beforeLine, lines}) =>
      Added({beforeLine: beforeLine + idx, lines})
    | Deleted({startLine, stopLine}) =>
      Deleted({startLine: startLine + idx, stopLine: stopLine + idx})
    | Modified({line, original, updated}) =>
      Modified({line: line + idx, original, updated})
    }
  );
};

[@deriving show]
type t = list(update);

let map = List.map;

let toDebugString = show;

module Internal = {
  let compute = (~original, ~updated) => {
    let originalLineCount = Array.length(original);
    let updatedLineCount = Array.length(updated);

    let (deletes, adds) = Diff.f(original, updated);

    let isAdded = i => {
      i < updatedLineCount && adds[i];
    };

    let isDeleted = i => {
      i < originalLineCount && deletes[i];
    };

    // Return the number of 'true' lines, after `idx`
    let numberOfTrue = (idx, array) => {
      let len = Array.length(array);

      let rec loop = (acc, curr) =>
        if (curr >= len) {
          acc;
        } else if (array[curr]) {
          loop(acc + 1, curr + 1);
        } else {
          acc;
        };

      loop(0, idx);
    };

    // TODO: Fix recursion
    // Maybe track index for original and udpated separately?
    let rec loop = (acc, originalIdx, updatedIdx) =>
      if (updatedIdx >= updatedLineCount && originalIdx >= originalLineCount) {
        // Reached end of both arrays, at the same time
        acc;
        // } else if (originalIdx >= originalLineCount && updatedIdx < updatedLineCount) {
        // TODO: Reached end of original lines, but still adds
        //   acc;
        // } else if (updatedIdx >= updatedLineCount
        //            && originalIdx < originalLineCount) {
        // TODO: Reached end up of updated lines, but still deletes
        //   acc;
      } else {
        switch (isDeleted(originalIdx), isAdded(updatedIdx)) {
        // Modified line
        | (true, true) =>
          let lineIdx = LineNumber.ofZeroBased(originalIdx);
          loop(
            [
              Modified({
                line: lineIdx,
                original: original[originalIdx],
                updated: updated[updatedIdx],
              }),
              ...acc,
            ],
            originalIdx + 1,
            updatedIdx + 1,
          );

        // Deleted line
        | (true, false) =>
          let lineIdx = LineNumber.ofZeroBased(originalIdx);
          let additionalDeletes = numberOfTrue(originalIdx, deletes);
          loop(
            [
              Deleted({
                startLine: lineIdx,
                stopLine: LineNumber.(lineIdx + additionalDeletes),
              }),
              ...acc,
            ],
            originalIdx + additionalDeletes,
            updatedIdx,
          );

        | (false, true) =>
          let additionalAdds = numberOfTrue(updatedIdx, adds);
          let lines = Array.sub(updated, updatedIdx, additionalAdds);

          let lineIdx = LineNumber.ofZeroBased(originalIdx);
          loop(
            [Added({beforeLine: lineIdx, lines}), ...acc],
            originalIdx,
            updatedIdx + additionalAdds,
          );

        | (false, false) => loop(acc, originalIdx + 1, updatedIdx + 1)
        };
      };

    loop([], 0, 0);
  };

  let%test_module "compute" =
    (module
     {
       let%test "empty arrays result in no updates" = {
         let original = [||];
         let updated = [||];
         compute(~original, ~updated) == [];
       };

       let%test "modification gives an edit" = {
         let original = [|"a"|];
         let updated = [|"b"|];
         compute(~original, ~updated)
         == [Modified({line: LineNumber.zero, original: "a", updated: "b"})];
       };

       let%test "delete line in middle of array" = {
         let original = [|"a", "b", "c"|];
         let updated = [|"a", "c"|];
         compute(~original, ~updated)
         == [
              Deleted({
                startLine: LineNumber.(zero + 1),
                stopLine: LineNumber.(zero + 2),
              }),
            ];
       };

       let%test "delete multiple lines in middle of array" = {
         let original = [|"a", "b", "c", "d"|];
         let updated = [|"a", "d"|];
         compute(~original, ~updated)
         == [
              Deleted({
                startLine: LineNumber.(zero + 1),
                stopLine: LineNumber.(zero + 3),
              }),
            ];
       };

       let%test "add line in middle of array" = {
         let original = [|"a", "c"|];
         let updated = [|"a", "b", "c"|];
         compute(~original, ~updated)
         == [Added({beforeLine: LineNumber.(zero + 1), lines: [|"b"|]})];
       };

       let%test "add multiple lines in middle of array" = {
         let original = [|"a", "d"|];
         let updated = [|"a", "b", "c", "d"|];
         compute(~original, ~updated)
         == [
              Added({beforeLine: LineNumber.(zero + 1), lines: [|"b", "c"|]}),
            ];
       };

       let%test "add single line to empty array" = {
         let original = [||];
         let updated = [|"a"|];
         compute(~original, ~updated)
         == [Added({beforeLine: LineNumber.(zero), lines: [|"a"|]})];
       };

       let%test "add multiple lines to empty array" = {
         let original = [||];
         let updated = [|"a", "b"|];
         compute(~original, ~updated)
         == [Added({beforeLine: LineNumber.(zero), lines: [|"a", "b"|]})];
       };

       let%test "remove single line to empty array" = {
         let original = [|"a"|];
         let updated = [||];
         compute(~original, ~updated)
         == [
              Deleted({
                startLine: LineNumber.(zero),
                stopLine: LineNumber.(zero + 1),
              }),
            ];
       };

       let%test "remove multiple lines to empty array" = {
         let original = [|"a", "b"|];
         let updated = [||];
         compute(~original, ~updated)
         == [
              Deleted({
                startLine: LineNumber.(zero),
                stopLine: LineNumber.(zero + 2),
              }),
            ];
       };

       let%test "remove separate lines" = {
         let original = [|"a", "b", "c", "d", "e"|];
         let updated = [|"a", "c", "e"|];
         compute(~original, ~updated)
         == [
              Deleted({
                startLine: LineNumber.(zero + 3),
                stopLine: LineNumber.(zero + 4),
              }),
              Deleted({
                startLine: LineNumber.(zero + 1),
                stopLine: LineNumber.(zero + 2),
              }),
            ];
       };

       let%test "add separate lines" = {
         let original = [|"a", "c", "e"|];
         let updated = [|"a", "b", "c", "d", "e"|];
         let ret = compute(~original, ~updated);
         ret
         == [
              Added({beforeLine: LineNumber.(zero + 2), lines: [|"d"|]}),
              Added({beforeLine: LineNumber.(zero + 1), lines: [|"b"|]}),
            ];
       };

       let%test "add separate, multiple lines" = {
         let original = [|"a", "c", "e"|];
         let updated = [|"a", "b0", "b1", "c", "d0", "d1", "e"|];
         let ret = compute(~original, ~updated);
         ret
         == [
              Added({
                beforeLine: LineNumber.(zero + 2),
                lines: [|"d0", "d1"|],
              }),
              Added({
                beforeLine: LineNumber.(zero + 1),
                lines: [|"b0", "b1"|],
              }),
            ];
       };

       let%test "delete some, add some" = {
         let original = [|"a", "b0", "b1", "c", "e"|];
         let updated = [|"a", "c", "d0", "d1", "e"|];
         let ret = compute(~original, ~updated);
         ret
         == [
              Added({
                beforeLine: LineNumber.(zero + 4),
                lines: [|"d0", "d1"|],
              }),
              Deleted({
                startLine: LineNumber.(zero + 1),
                stopLine: LineNumber.(zero + 3),
              }),
            ];
       };

       let%test "add some, delete some" = {
         let original = [|"a", "c", "d0", "d1", "e"|];
         let updated = [|"a", "b0", "b1", "c", "e"|];
         let ret = compute(~original, ~updated);
         ret
         == [
              Deleted({
                startLine: LineNumber.(zero + 2),
                stopLine: LineNumber.(zero + 4),
              }),
              Added({
                beforeLine: LineNumber.(zero + 1),
                lines: [|"b0", "b1"|],
              }),
            ];
       };
     });
};

let fromBuffers = (~original, ~updated) => {
  let originalLines = Buffer.getLines(original);
  let updatedLines = Buffer.getLines(updated);

  Internal.compute(~original=originalLines, ~updated=updatedLines);
};

let fromBufferUpdate = (~buffer: Buffer.t, ~update: BufferUpdate.t) => {
  let original =
    if (update.startLine == update.endLine) {
      [||];
    } else {
      let startIdx = EditorCoreTypes.LineNumber.toZeroBased(update.startLine);
      let stopIdx = EditorCoreTypes.LineNumber.toZeroBased(update.endLine);
      let lines = Buffer.getLines(buffer);
      Utility.ArrayEx.slice(
        ~lines,
        ~start=startIdx,
        ~length=stopIdx - startIdx,
        (),
      );
    };

  let updated = update.lines;

  Internal.compute(~original, ~updated) |> List.map(shift(update.startLine));
};
