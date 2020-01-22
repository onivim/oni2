open EditorCoreTypes;
open Oni_Core;

exception OptionInvalidException(string);
let getOrThrow: option('a) => 'a =
  v =>
    switch (v) {
    | Some(v) => v
    | None => raise(OptionInvalidException("Excepted 'Some' but got 'None'"))
    };

let getOrThrowResult: result('a, 'b) => 'a =
  v =>
    switch (v) {
    | Ok(v) => v
    | Error(e) => failwith(e)
    };

let repeat = (~iterations: int=5, f) => {
  let count = ref(0);

  while (count^ < iterations) {
    f();
    count := count^ + 1;
  };
};

let validateRange =
    (
      expect: Rely__DefaultMatchers.matchers(unit),
      actualRange: Range.t,
      expectedRange: Range.t,
    ) => {
  expect.int(Index.toZeroBased(actualRange.start.line)).toBe(
    Index.toZeroBased(expectedRange.start.line),
  );
  expect.int(Index.toZeroBased(actualRange.stop.line)).toBe(
    Index.toZeroBased(expectedRange.stop.line),
  );
  expect.int(Index.toZeroBased(actualRange.start.column)).toBe(
    Index.toZeroBased(expectedRange.start.column),
  );
  expect.int(Index.toZeroBased(actualRange.stop.column)).toBe(
    Index.toZeroBased(expectedRange.stop.column),
  );
};

let validateRanges =
    (
      expect: Rely__DefaultMatchers.matchers(unit),
      actualRanges,
      expectedRanges,
    ) => {
  List.iter2(validateRange(expect), actualRanges, expectedRanges);
};

let validateBuffer =
    (
      expect: Rely__DefaultMatchers.matchers(unit),
      actualBuffer: Buffer.t,
      expectedLines: array(string),
    ) => {
  expect.int(Buffer.getNumberOfLines(actualBuffer)).toBe(
    Array.length(expectedLines),
  );

  let validateLine = (actualLine, expectedLine) => {
    expect.string(actualLine).toEqual(expectedLine);
  };

  let f = (i, expected) => {
    let actual = Buffer.getLine(i, actualBuffer) |> Buffer.BufferLine.slowGetString;
    validateLine(actual, expected);
  };

  Array.iteri(f, expectedLines);
};
