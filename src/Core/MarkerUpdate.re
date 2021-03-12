open EditorCoreTypes;

type movement =
  | Noop
  | DeleteLines({
      startLine: LineNumber.t,
      stopLine: LineNumber.t,
    })
  | ShiftLines({
      afterLine: LineNumber.t,
      delta: int,
    })
  | ShiftCharacters({
      line: LineNumber.t,
      afterByte: ByteIndex.t,
      afterCharacter: CharacterIndex.t,
      deltaBytes: int,
      deltaCharacters: int,
    });

type t = list(movement);

module Internal = {
  let minimalUpdateToMovement: MinimalUpdate.update => movement =
    update => {
      MinimalUpdate.(
        switch (update) {
        | Added({beforeLine, lines}) =>
          ShiftLines({afterLine: beforeLine, delta: Array.length(lines)})

        | Deleted({startLine, stopLine}) =>
          DeleteLines({startLine, stopLine})

        | Modified({line, original, updated}) =>
          let deltaBytes = String.length(updated) - String.length(original);

          // For now, we'll only handle insertions / deletions
          if (deltaBytes == 0) {
            Noop;
          } else {
            Utility.StringEx.firstDifference(original, updated)
            |> Option.map(firstDifferentByte => {
                 let characterCount =
                   Utility.StringEx.characterCount(
                     ~startByte=0,
                     ~endByte=firstDifferentByte,
                     original,
                   );

                 // If delta bytes is less than 0 - ie, bytes were removed,
                 // need to look at the original line
                 let deltaCharacterCount =
                   if (deltaBytes < 0) {
                     Utility.StringEx.characterCount(
                       ~startByte=firstDifferentByte,
                       ~endByte=firstDifferentByte + abs(deltaBytes),
                       original,
                     )
                     * (-1);
                   } else {
                     Utility.StringEx.characterCount(
                       ~startByte=firstDifferentByte,
                       ~endByte=firstDifferentByte + deltaBytes,
                       updated,
                     );
                   };

                 ShiftCharacters({
                   line,
                   afterByte: ByteIndex.ofInt(firstDifferentByte),
                   deltaBytes,
                   afterCharacter: CharacterIndex.ofInt(characterCount),
                   deltaCharacters: deltaCharacterCount,
                 });
               })
            |> Option.value(~default=Noop);
          };
        }
      );
    };
};

let create = (~update: BufferUpdate.t, ~original, ~updated) =>
  if (update.isFull) {
    let minimalUpdate = MinimalUpdate.fromBuffers(~original, ~updated);

    MinimalUpdate.map(Internal.minimalUpdateToMovement, minimalUpdate);
  } else {
    let minimalUpdate =
      MinimalUpdate.fromBufferUpdate(~buffer=original, ~update);

    MinimalUpdate.map(Internal.minimalUpdateToMovement, minimalUpdate);
  };

let apply = (~clearLine, ~shiftLines, ~shiftCharacters, markerUpdate, target) => {
  markerUpdate
  |> List.fold_left(
       (acc, movement) => {
         switch (movement) {
         | Noop => acc

         | ShiftLines({afterLine, delta}) =>
           shiftLines(~afterLine, ~delta, acc)

         | DeleteLines({startLine, stopLine}) =>
           let delta =
             LineNumber.toZeroBased(stopLine)
             - LineNumber.toZeroBased(startLine);

           let rec clear = (acc, idx) =>
             if (idx >= delta) {
               acc;
             } else {
               let acc' = clearLine(~line=LineNumber.(startLine + idx), acc);
               clear(acc', idx + 1);
             };

           let acc' = clear(acc, 0);
           shiftLines(~afterLine=startLine, ~delta=- delta, acc');

         | ShiftCharacters({
             line,
             afterByte,
             afterCharacter,
             deltaBytes,
             deltaCharacters,
           }) =>
           shiftCharacters(
             ~line,
             ~afterByte,
             ~deltaBytes,
             ~afterCharacter,
             ~deltaCharacters,
             acc,
           )
         }
       },
       target,
     );
};
