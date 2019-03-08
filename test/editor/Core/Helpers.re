open Oni_Core;
open Oni_Core.Types;

module Buffer = Oni_Core.Buffer;

exception OptionInvalidException(string);
let getOrThrow: option('a) => 'a =
  v =>
    switch (v) {
    | Some(v) => v
    | None => raise(OptionInvalidException("Excepted 'Some' but got 'None'"))
    };

let repeat = (~iterations: int=5, f) => {
  let count = ref(0);

  while (count^ < iterations) {
    f();
    count := count^ + 1;
  };
};

let validateToken =
    (
      expect: Rely__DefaultMatchers.matchers(unit),
      actualToken: Tokenizer.t,
      expectedToken: Tokenizer.t,
    ) => {
  expect.string(actualToken.text).toEqual(expectedToken.text);
  expect.int(Index.toZeroBasedInt(actualToken.startPosition)).toBe(
    Index.toZeroBasedInt(expectedToken.startPosition),
  );
  expect.int(Index.toZeroBasedInt(actualToken.endPosition)).toBe(
    Index.toZeroBasedInt(expectedToken.endPosition),
  );
};

let validateTokens =
    (
      expect: Rely__DefaultMatchers.matchers(unit),
      actualTokens: list(Tokenizer.t),
      expectedTokens: list(Tokenizer.t),
    ) => {
  expect.int(List.length(actualTokens)).toBe(List.length(expectedTokens));

  let f = (actual, expected) => {
    validateToken(expect, actual, expected);
  };

  List.iter2(f, actualTokens, expectedTokens);
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
    let actual = Buffer.getLine(actualBuffer, i);
    validateLine(actual, expected);
  };

  Array.iteri(f, expectedLines);
};
