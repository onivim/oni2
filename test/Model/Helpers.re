open Oni_Core;
open Oni_Model;

let validateToken =
    (
      expect: Rely__DefaultMatchers.matchers(unit),
      actualToken: BufferViewTokenizer.t,
      expectedToken: BufferViewTokenizer.t,
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
      actualTokens: list(BufferViewTokenizer.t),
      expectedTokens: list(BufferViewTokenizer.t),
    ) => {
  expect.int(List.length(actualTokens)).toBe(List.length(expectedTokens));

  let f = (actual, expected) => {
    validateToken(expect, actual, expected);
  };

  List.iter2(f, actualTokens, expectedTokens);
};
