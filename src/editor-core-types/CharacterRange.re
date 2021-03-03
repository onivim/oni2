[@deriving show]
type t = {
  start: CharacterPosition.t,
  stop: CharacterPosition.t,
};

let explode = (measure, range) =>
  if (range.start.line == range.stop.line) {
    [range];
  } else {
    let ranges = ref([]);
    let pushRange = (~start, ~stop) =>
      ranges := [{start, stop}, ...ranges^];

    let i = ref(range.start.line);
    while (i^ < range.stop.line) {
      let startColumn =
        i^ == range.start.line ? range.start.character : CharacterIndex.zero;
      // TODO: Is this correct?
      let stopColumn = measure(i^) |> (v => CharacterIndex.ofInt(v + 1));

      pushRange(
        ~start=CharacterPosition.{line: i^, character: startColumn},
        ~stop=CharacterPosition.{line: i^, character: stopColumn},
      );

      i := LineNumber.(i^ + 1);
    };

    pushRange(
      ~start=
        CharacterPosition.{
          line: range.stop.line,
          character: CharacterIndex.zero,
        },
      ~stop=
        CharacterPosition.{
          line: range.stop.line,
          character: range.stop.character,
        },
    );

    ranges^ |> List.rev;
  };

let contains = (position: CharacterPosition.t, range) => {
  (
    LineNumber.(position.line == range.start.line)
    && CharacterIndex.(position.character >= range.start.character)
    || LineNumber.(position.line > range.start.line)
  )
  && (
    LineNumber.(position.line == range.stop.line)
    && CharacterIndex.(position.character <= range.stop.character)
    || LineNumber.(position.line < range.stop.line)
  );
};

let shiftLine = (~afterLine: LineNumber.t, ~delta, range) => {
  LineNumber.(
    {
      let start' =
        if (range.start.line >= afterLine) {
          {...range.start, line: range.start.line + delta};
        } else {
          range.start;
        };

      let stop' =
        if (range.stop.line >= afterLine) {
          {...range.stop, line: range.stop.line + delta};
        } else {
          range.stop;
        };
      {start: start', stop: stop'};
    }
  );
};

let shiftCharacters = (~line, ~afterCharacter, ~delta, range) => {
  CharacterIndex.(
    {
      let start' =
        if (range.start.line == line && range.start.character >= afterCharacter) {
          {...range.start, character: range.start.character + delta};
        } else {
          range.start;
        };

      let stop' =
        if (range.stop.line == line && range.stop.character >= afterCharacter) {
          {...range.stop, character: range.stop.character + delta};
        } else {
          range.stop;
        };
      {start: start', stop: stop'};
    }
  );
};

let toHash = ranges => {
  let hash = Hashtbl.create(100);

  List.iter(
    range => {
      let line = range.start.line;
      let lineRanges =
        switch (Hashtbl.find_opt(hash, line)) {
        | Some(ranges) => ranges
        | None => []
        };

      Hashtbl.add(hash, line, [range, ...lineRanges]);
    },
    ranges,
  );

  hash;
};

let equals = (a, b) =>
  CharacterPosition.(a.start == b.start && a.stop == b.stop);
