open EditorCoreTypes;
open Oni_Core;
open Oni_Core.Utility;

module Constants = {
  // TODO: Tune to ensure acceptable perf
  let maxTravel = 10000;
  let maxLineLength = 1000;
};

type pair = {
  start: CharacterPosition.t,
  stop: CharacterPosition.t,
};

let find =
    (
      ~buffer: EditorBuffer.t,
      ~characterPosition: CharacterPosition.t,
      ~start: Uchar.t,
      ~stop: Uchar.t,
    ) => {
  let bufferLen = EditorBuffer.numberOfLines(buffer);

  let line = characterPosition.line |> EditorCoreTypes.LineNumber.toZeroBased;
  let index = characterPosition.character |> CharacterIndex.toInt;

  let rec loop =
          (
            ~index=None,
            ~direction,
            ~count,
            ~line,
            ~travel,
            startCharacter,
            stopCharacter,
          ) =>
    // Traveled as far as we could, didn't find a pair...
    if (travel > Constants.maxTravel) {
      None;
          // Hit limits of buffer, didn't find a pair
    } else if (line < 0) {
      None;
    } else if (line >= bufferLen) {
      None;
    } else {
      let bufferLine = buffer |> EditorBuffer.line(line);
      let lineLength =
        bufferLine
        |> BufferLine.lengthBounded(
             ~max=Constants.maxLineLength |> CharacterIndex.ofInt,
           );

      let idx =
        switch (index) {
        | None => direction > 0 ? 0 : lineLength - 1
        | Some(i) => i
        };

      if (idx < 0) {
        loop(
          ~direction,
          ~count,
          ~line=line + direction,
          ~index=None,
          ~travel=travel + 1,
          startCharacter,
          stopCharacter,
        );
      } else if (idx >= lineLength) {
        loop(
          ~direction,
          ~count,
          ~line=line + direction,
          ~index=None,
          ~travel=travel + 1,
          startCharacter,
          stopCharacter,
        );
      } else {
        let char: Uchar.t =
          bufferLine
          |> BufferLine.getUcharExn(~index=idx |> CharacterIndex.ofInt);

        let count =
          if (char == startCharacter) {
            count + direction;
          } else if (char == stopCharacter) {
            count - direction;
          } else {
            count;
          };

        if (count == 0) {
          Some(
            {
              line: EditorCoreTypes.LineNumber.ofZeroBased(line),
              character: CharacterIndex.ofInt(idx),
            }: CharacterPosition.t,
          );
        } else {
          loop(
            ~count,
            ~direction,
            ~line,
            ~index=Some(idx + direction),
            ~travel=travel + 1,
            startCharacter,
            stopCharacter,
          );
        };
      };
    };

  // If we're starting on the 'stop' character, rewind back once
  // so we pick up that particular pair.
  // A bunch of tedious logic to rewind a character safely, crossing line boundaries:
  let (index', line') =
    if (line < bufferLen && line >= 0) {
      let bufferLine = buffer |> EditorBuffer.line(line);
      let lineLength =
        bufferLine
        |> BufferLine.lengthBounded(
             ~max=Constants.maxLineLength |> CharacterIndex.ofInt,
           );

      if (index < lineLength && index >= 0) {
        let char: Uchar.t =
          bufferLine
          |> BufferLine.getUcharExn(~index=CharacterIndex.ofInt(index));
        if (char != stop) {
          (Some(index), line);
        } else if (index > 0) {
          (Some(index - 1), line);
        } else if (line > 0) {
          (None, line - 1);
        } else {
          (Some(index), line);
        };
      } else {
        (Some(index), line);
      };
    } else {
      (Some(index), line);
    };

  // See if we can find the 'start' bracket
  let maybeStart =
    loop(
      ~count=1,
      ~line=line',
      ~index=index',
      ~travel=0,
      ~direction=-1,
      start,
      stop,
    );

  maybeStart
  |> OptionEx.flatMap(startPos => {
       let {line, character}: CharacterPosition.t = startPos;

       loop(
         ~count=0,
         ~line=EditorCoreTypes.LineNumber.toZeroBased(line),
         ~index=Some(character |> CharacterIndex.toInt),
         ~travel=0,
         ~direction=1,
         start,
         stop,
       )
       |> Option.map(stop => {start: startPos, stop});
     });
};

let findFirst = (~buffer, ~characterPosition, ~pairs) => {
  List.fold_left(
    (acc, pair) => {
      let LanguageConfiguration.BracketPair.{openPair, closePair} = pair;

      // TODO: Handle longer brackets
      if (String.length(openPair) != 1 || String.length(closePair) != 1) {
        acc;
      } else {
        let startCharacter = Uchar.of_char(openPair.[0]);
        let stopCharacter = Uchar.of_char(closePair.[0]);

        let maybePair =
          find(
            ~buffer,
            ~characterPosition,
            ~start=startCharacter,
            ~stop=stopCharacter,
          );

        switch (acc, maybePair) {
        | (Some(_) as acc, None) => acc
        | (None, Some(_) as newPair) => newPair
        | (Some(previousPair), Some(newPair)) =>
          let newStartLine =
            newPair.start.line |> EditorCoreTypes.LineNumber.toZeroBased;
          let previousStartLine =
            previousPair.start.line |> EditorCoreTypes.LineNumber.toZeroBased;
          let previousStartIndex =
            previousPair.start.character |> CharacterIndex.toInt;
          let newStartIndex = newPair.start.character |> CharacterIndex.toInt;
          if (newStartLine > previousStartLine
              || newStartLine == previousStartLine
              && newStartIndex > previousStartIndex) {
            Some(newPair);
          } else {
            Some(previousPair);
          };
        | (None, None) => None
        };
      };
    },
    None,
    pairs,
  );
};
