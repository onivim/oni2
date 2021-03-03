open EditorCoreTypes;

type movement =
  | ClearLine({line: LineNumber.t})
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

let none = [];

let create = (~update: BufferUpdate.t, ~original, ~updated) =>
  if (update.isFull) {
    none;
  } else {
    let startPos = update.startLine |> EditorCoreTypes.LineNumber.toZeroBased;
    let endPos = update.endLine |> EditorCoreTypes.LineNumber.toZeroBased;
    let len = Array.length(update.lines);
    let delta = Array.length(update.lines) - (endPos - startPos);
    if (delta == 0
        && len == 1
        && Buffer.getNumberOfLines(original) > startPos
        && Buffer.getNumberOfLines(updated) > startPos) {
      // Single line was updated - we can adjust the bytes / characters in the line
      let originalLine =
        original |> Buffer.getLine(startPos) |> BufferLine.raw;

      let newLine = updated |> Buffer.getLine(startPos) |> BufferLine.raw;

      let deltaBytes = String.length(newLine) - String.length(originalLine);

      // For now, we'll only handle insertions / deletions
      if (deltaBytes == 0) {
        none;
      } else {
        Utility.StringEx.firstDifference(originalLine, newLine)
        |> Option.map(firstDifferentByte => {
             let characterCount =
               Utility.StringEx.characterCount(
                 ~startByte=0,
                 ~endByte=firstDifferentByte,
                 originalLine,
               );

             // If delta bytes is less than 0 - ie, bytes were removed,
             // need to look at the original line
             let deltaCharacterCount =
               if (deltaBytes < 0) {
                 Utility.StringEx.characterCount(
                   ~startByte=firstDifferentByte,
                   ~endByte=firstDifferentByte + abs(deltaBytes),
                   originalLine,
                 )
                 * (-1);
               } else {
                 Utility.StringEx.characterCount(
                   ~startByte=firstDifferentByte,
                   ~endByte=firstDifferentByte + deltaBytes,
                   newLine,
                 );
               };

             ShiftCharacters({
               line: update.startLine,
               afterByte: ByteIndex.ofInt(firstDifferentByte),
               deltaBytes,
               afterCharacter: CharacterIndex.ofInt(characterCount),
               deltaCharacters: deltaCharacterCount,
             });
           })
        |> Option.to_list;
      };
    } else if (delta != 0) {
      let afterLine = delta < 0 ? update.startLine : update.endLine;
      let candidate = ShiftLines({afterLine, delta});

      // Pure delete - let's clear out the lines in the update
      if (delta < 0 && Array.length(update.lines) == 0) {
        let linesToClear =
          List.init(abs(delta), i =>
            ClearLine({line: LineNumber.(update.startLine + i)})
          );

        [candidate, ...linesToClear] |> List.rev;
      } else {
        [candidate];
      };
    } else {
      none;
    };
  };

let apply = (~clearLine, ~shiftLines, ~shiftCharacters, markerUpdate, target) => {
  markerUpdate
  |> List.fold_left(
       (acc, movement) => {
         switch (movement) {
         | ClearLine({line}) => clearLine(~line, acc)

         | ShiftLines({afterLine, delta}) =>
           shiftLines(~afterLine, ~delta, acc)

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
