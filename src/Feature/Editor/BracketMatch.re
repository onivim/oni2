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

  let rec loop =
          (
            ~direction,
            ~count,
            ~line,
            ~index=None,
            ~travel,
            ~startCharacter,
            ~stopCharacter,
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
        bufferLine |> BufferLine.lengthBounded(~max=Constants.maxLineLength);

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
          ~startCharacter,
          ~stopCharacter,
        );
      } else if (idx >= lineLength) {
        loop(
          ~direction,
          ~count,
          ~line=line + direction,
          ~index=None,
          ~travel=travel + 1,
          ~startCharacter,
          ~stopCharacter,
        );
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

  // If we're starting on the 'stop' character, rewind back once
  // so we pick up that particular pair.
  // A bunch of tedious logic to rewind a character safely, crossing line boundaries:
  let (index', line') =
    if (line < bufferLen && line >= 0) {
      let bufferLine = buffer |> EditorBuffer.line(line);
      let lineLength =
        bufferLine |> BufferLine.lengthBounded(~max=Constants.maxLineLength);

      if (index < lineLength && index >= 0) {
        let char: Uchar.t = bufferLine |> BufferLine.getUcharExn(~index);
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
      ~startCharacter=start,
      ~stopCharacter=stop,
    );

  maybeStart
  |> OptionEx.flatMap(startPos => {
       let {line, index} = startPos;

       loop(
         ~count=0,
         ~line,
         ~index=Some(index),
         ~travel=0,
         ~direction=1,
         ~startCharacter=start,
         ~stopCharacter=stop,
       )
       |> Option.map(stop => {start: startPos, stop});
     });
};

let findFirst = (~buffer, ~line, ~index, ~pairs) => {
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
            ~line,
            ~index,
            ~start=startCharacter,
            ~stop=stopCharacter,
          );

        switch (acc, maybePair) {
        | (Some(_) as acc, None) => acc
        | (None, Some(_) as newPair) => newPair
        | (Some(previousPair), Some(newPair)) =>
          if (newPair.start.line > previousPair.start.line
              || newPair.start.line == previousPair.start.line
              && newPair.start.index > previousPair.start.index) {
            Some(newPair);
          } else {
            Some(previousPair);
          }
        | (None, None) => None
        };
      };
    },
    None,
    pairs,
  );
};

module Test = {
  let create = lines =>
    lines |> Array.of_list |> Oni_Core.Buffer.ofLines |> EditorBuffer.ofBuffer;
  let%test_module "findFirst" =
    (module
     {
       let pairs =
         LanguageConfiguration.BracketPair.[
           {openPair: "{", closePair: "}"},
           {openPair: "(", closePair: ")"},
           {openPair: "[", closePair: "]"},
         ];

       let%test "empty buffer" = {
         let buffer = create([""]);

         None == findFirst(~buffer, ~line=0, ~index=0, ~pairs);
       };
       let%test "pair outside should not get picked up" = {
         let buffer = create(["abc", "", "({[a]})"]);

         None == findFirst(~buffer, ~line=1, ~index=0, ~pairs);
       };
       let%test "picks nearest" = {
         let buffer = create(["({[a]})"]);

         Some({
           start: {
             line: 0,
             index: 2,
           },
           stop: {
             line: 0,
             index: 4,
           },
         })
         == findFirst(~buffer, ~line=0, ~index=3, ~pairs);
       };
       let%test "picks nearest, outside" = {
         let buffer = create(["({[a]})"]);

         Some({
           start: {
             line: 0,
             index: 1,
           },
           stop: {
             line: 0,
             index: 5,
           },
         })
         == findFirst(~buffer, ~line=0, ~index=1, ~pairs);
       };
       let%test "uses closing character if cursor is on it" = {
         let buffer = create(["({[a]})"]);

         Some({
           start: {
             line: 0,
             index: 2,
           },
           stop: {
             line: 0,
             index: 4,
           },
         })
         == findFirst(~buffer, ~line=0, ~index=4, ~pairs);
       };
     });

  let%test_module "find" =
    (module
     {
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

         Some({
           start: {
             line: 0,
             index: 0,
           },
           stop: {
             line: 0,
             index: 1,
           },
         })
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

         Some({
           start: {
             line: 0,
             index: 0,
           },
           stop: {
             line: 0,
             index: 2,
           },
         })
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

         Some({
           start: {
             line: 0,
             index: 1,
           },
           stop: {
             line: 2,
             index: 0,
           },
         })
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

         Some({
           start: {
             line: 0,
             index: 0,
           },
           stop: {
             line: 2,
             index: 3,
           },
         })
         == find(
              ~buffer,
              ~line=1,
              ~index=0,
              ~start=leftBracket,
              ~stop=rightBracket,
            );
       };
     });
};
