open Oni_Core;
open Oni_Core.Types;

open Oni_Model;

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
