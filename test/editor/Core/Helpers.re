open Oni_Core;
open Oni_Core.Types;

let validateToken =
    (
      expect: Rely__DefaultMatchers.matchers(string),
      actualToken: Tokenizer.t,
      expectedToken: Tokenizer.t,
    ) => {
  expect.string(actualToken.text).toEqual(expectedToken.text);
  expect.int(Position.toZeroBasedIndex(actualToken.startPosition)).toBe(
    Position.toZeroBasedIndex(expectedToken.startPosition),
  );
  expect.int(Position.toZeroBasedIndex(actualToken.endPosition)).toBe(
    Position.toZeroBasedIndex(expectedToken.endPosition),
  );
};

let validateTokens =
    (
      expect: Rely__DefaultMatchers.matchers(string),
      actualTokens: list(Tokenizer.t),
      expectedTokens: list(Tokenizer.t),
    ) => {
  expect.int(List.length(actualTokens)).toBe(List.length(expectedTokens));

  let f = (actual, expected) => {
    validateToken(expect, actual, expected);
  };

  List.iter2(f, actualTokens, expectedTokens);
};
