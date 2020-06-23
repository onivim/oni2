open Oni_Core;
type direction =
  | Forwards
  | Backwards;

module Constants = {
  // TODO: Tune to ensure acceptable perf
  let maxTravel = 10000;
  let maxLineLength = 1000;
};

type t = {
  line: int,
  index: int,
};

let matchingPair =
    (
      ~buffer: EditorBuffer.t,
      ~line: int,
      ~index: int,
      ~direction: direction,
      ~current: Uchar.t,
      ~destination: Uchar.t,
    ) => {
  let bufferLen = EditorBuffer.numberOfLines(buffer);

  let direction = direction == Forwards ? 1 : (-1);

  let rec loop = (~count, ~line, ~index=None, ~travel) =>
    // Traveled as far as we could, didn't find a pair...
    if (travel > Constants.maxTravel) {
      None;
          // Hit limits of buffer, didn't find a pair
    } else if (line < 0) {
      None;
    } else if (line >= bufferLen) {
      None;
    } else {
      let idx =
        switch (index) {
        | None => 0
        | Some(i) => i
        };

      let bufferLine = buffer |> EditorBuffer.line(line);
      let lineLength =
        bufferLine |> BufferLine.lengthBounded(~max=Constants.maxTravel);

      if (idx >= lineLength) {
        loop(~count, ~line=line + 1, ~index=None, ~travel=travel + 1);
      } else {
        let char: Uchar.t = bufferLine |> BufferLine.getUcharExn(~index=idx);

        let count =
          if (char == current) {
            count + 1;
          } else if (char == destination) {
            count - 1;
          } else {
            count;
          };

        if (count == 0) {
          Some({line, index: idx});
        } else {
          loop(
            ~count,
            ~line,
            ~index=Some(idx + direction),
            ~travel=travel + 1,
          );
        };
      };
    };

  loop(~count=1, ~line, ~index=Some(index + direction), ~travel=0);
};
