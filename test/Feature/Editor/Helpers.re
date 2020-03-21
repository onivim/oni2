open EditorCoreTypes;
open Feature_Editor;

let validateToken =
    (
      expect: Rely.matchers(unit),
      actualToken: BufferViewTokenizer.t,
      expectedToken: BufferViewTokenizer.t,
    ) => {
  expect.string(actualToken.text).toEqual(expectedToken.text);
  expect.int(Index.toZeroBased(actualToken.startPosition)).toBe(
    Index.toZeroBased(expectedToken.startPosition),
  );
  expect.int(Index.toZeroBased(actualToken.endPosition)).toBe(
    Index.toZeroBased(expectedToken.endPosition),
  );
};

let validateTokens =
    (
      expect: Rely.matchers(unit),
      actualTokens: list(BufferViewTokenizer.t),
      expectedTokens: list(BufferViewTokenizer.t),
    ) => {
  expect.int(List.length(actualTokens)).toBe(List.length(expectedTokens));

  let f = (actual, expected) => {
    validateToken(expect, actual, expected);
  };

  List.iter2(f, actualTokens, expectedTokens);
};
