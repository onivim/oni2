open Oni_Core;
open Oni_Core.Types;

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
  expect.int(Index.toZeroBasedInt(actualRange.startPosition.line)).toBe(
    Index.toZeroBasedInt(expectedRange.startPosition.line),
  );
  expect.int(Index.toZeroBasedInt(actualRange.endPosition.line)).toBe(
    Index.toZeroBasedInt(expectedRange.endPosition.line),
  );
  expect.int(Index.toZeroBasedInt(actualRange.startPosition.character)).toBe(
    Index.toZeroBasedInt(expectedRange.startPosition.character),
  );
  expect.int(Index.toZeroBasedInt(actualRange.endPosition.character)).toBe(
    Index.toZeroBasedInt(expectedRange.endPosition.character),
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
