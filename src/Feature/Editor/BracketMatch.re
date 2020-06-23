open Oni_Core;
open Oni_Core.Utility;

module Constants = {
  // TODO: Tune to ensure acceptable perf
  let maxTravel = 10000;
  let maxLineLength = 1000;
};

type position = {
  line: int,
  index: int,
};

type pair = {
  start: position,
  stop: position,
};

let find =
    (
      ~buffer: EditorBuffer.t,
      ~line: int,
      ~index: int,
      ~start: Uchar.t,
      ~stop: Uchar.t,
    ) => {
  let bufferLen = EditorBuffer.numberOfLines(buffer);

  let rec loop = (~direction, ~count, ~line, ~index=None, ~travel, ~startCharacter, ~stopCharacter) =>
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
        bufferLine |> BufferLine.lengthBounded(~max=Constants.maxLineLength);

      let idx =
        switch (index) {
        | None => direction > 0 ? 0 : lineLength - 1
        | Some(i) => i
        };

      if (idx < 0) {
        loop(~direction, ~count, ~line=line - 1, ~index=None, ~travel=travel + 1, ~startCharacter, ~stopCharacter);
      } else if (idx >= lineLength) {
        loop(~direction, ~count, ~line=line + 1, ~index=None, ~travel=travel + 1, ~startCharacter, ~stopCharacter);
      } else {
        let char: Uchar.t = bufferLine |> BufferLine.getUcharExn(~index=idx);

        let count =
          if (char == startCharacter) {
            count + direction;
          } else if (char == stopCharacter) {
            count - direction;
          } else {
            count;
          };

        if (count == 0) {
          Some({line, index: idx});
        } else {
          loop(
            ~count,
            ~direction,
            ~line,
            ~index=Some(idx + direction),
            ~travel=travel + 1,
            ~startCharacter,
            ~stopCharacter,
          );
        };
      };
    };

  // See if we can find the 'start' bracket

  let maybeStart = loop(~count=1, ~line, ~index=Some(index), ~travel=0, ~direction=-1, ~startCharacter=start, ~stopCharacter=stop);

  maybeStart
  |> OptionEx.flatMap(startPos => {
      let {line, index} = startPos;
    
    loop(~count=0, ~line, ~index=Some(index), ~travel=0, ~direction=1,~startCharacter=start, ~stopCharacter=stop)
    |> Option.map(stop => {
      start: startPos,
      stop
    });
  });
};

let%test_module "find" =
  (module
   {
     let create = lines =>
       lines
       |> Array.of_list
       |> Oni_Core.Buffer.ofLines
       |> EditorBuffer.ofBuffer;

     let leftBracket = Uchar.of_char('{');
     let rightBracket = Uchar.of_char('}');

     let%test "empty buffer" = {
       let buffer = create([""]);

       None
       == find(
            ~buffer,
            ~line=0,
            ~index=0,
            ~start=Uchar.of_int(0),
            ~stop=Uchar.of_int(0),
          );
     };
     let%test "out of bounds: line < 0" = {
       let buffer = create([""]);

       None
       == find(
            ~buffer,
            ~line=-1,
            ~index=0,
            ~start=Uchar.of_int(0),
            ~stop=Uchar.of_int(0),
          );
     };
     let%test "out of bounds: line > len0" = {
       let buffer = create([""]);

       None
       == find(
            ~buffer,
            ~line=2,
            ~index=0,
            ~start=Uchar.of_int(0),
            ~stop=Uchar.of_int(0),
          );
     };
     let%test "find pair at start position" = {
       let buffer = create(["{}"]);

       Some({start: { line: 0, index: 0}, stop: {line: 0, index: 1}})
       == find(
            ~buffer,
            ~line=0,
            ~index=0,
            ~start=leftBracket,
            ~stop=rightBracket,
          );
     };
     let%test "find before start position" = {
       let buffer = create(["{a}"]);

       Some({start: { line: 0, index: 0}, stop: {line: 0, index: 2}})
       == find(
            ~buffer,
            ~line=0,
            ~index=1,
            ~start=leftBracket,
            ~stop=rightBracket,
          );
     };
     let%test "find before/after line" = {
       let buffer = create(["a{", "bc", "}d"]);

       Some({start: { line: 0, index: 1}, stop: {line: 2, index: 0}})
       == find(
            ~buffer,
            ~line=1,
            ~index=1,
            ~start=leftBracket,
            ~stop=rightBracket,
          );
     };
     let%test "skip nested" = {
       let buffer = create(["{a{}", "bc", "{}d}"]);

       Some({start: { line: 0, index: 0}, stop: {line: 2, index: 3}})
       == find(
            ~buffer,
            ~line=1,
            ~index=0,
            ~start=leftBracket,
            ~stop=rightBracket,
          );
     };
   });
