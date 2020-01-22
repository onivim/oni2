open EditorCoreTypes;
open Oni_Core;
open TestFramework;

open CamomileLibrary;

module TextRun = Tokenizer.TextRun;

let alwaysSplit = (_, _, _, _) => true;
let noSplit = (_, _, _, _) => false;
let splitOnCharacter = (_, c1, _, c2) => !UChar.eq(c1, c2);

let thickB = c =>
  switch (UChar.char_of(c)) {
  | 'b' => 2
  | _ => 1
  };

let makeLine = str =>
  BufferLine.make(~indentation=IndentationSettings.default, str);

let validateToken =
    (
      expect: Rely__DefaultMatchers.matchers(unit),
      actualToken: TextRun.t,
      expectedToken: TextRun.t,
    ) => {
  expect.string(actualToken.text).toEqual(expectedToken.text);
  expect.int(Index.toZeroBased(actualToken.startIndex)).toBe(
    Index.toZeroBased(expectedToken.startIndex),
  );
  expect.int(Index.toZeroBased(actualToken.endIndex)).toBe(
    Index.toZeroBased(expectedToken.endIndex),
  );
  expect.int(Index.toZeroBased(actualToken.startPosition)).toBe(
    Index.toZeroBased(expectedToken.startPosition),
  );
  expect.int(Index.toZeroBased(actualToken.endPosition)).toBe(
    Index.toZeroBased(expectedToken.endPosition),
  );
};

let validateTokens =
    (
      expect: Rely__DefaultMatchers.matchers(unit),
      actualTokens: list(TextRun.t),
      expectedTokens: list(TextRun.t),
    ) => {
  expect.int(List.length(actualTokens)).toBe(List.length(expectedTokens));

  let f = (actual, expected) => {
    validateToken(expect, actual, expected);
  };

  List.iter2(f, actualTokens, expectedTokens);
};

describe("Tokenizer", ({test, describe, _}) => {
  describe("start / end indices", ({test, _}) => {
    test("empty string returns nothing", ({expect}) => {
      let measure = _ => 1;

      let result =
        Tokenizer.tokenize(
          ~startIndex=1,
          ~endIndex=3,
          ~f=noSplit,
          ~measure,
          "" |> makeLine,
        );

      let runs = [];

      validateTokens(expect, result, runs);
    });
    test("start index past string", ({expect}) => {
      let measure = _ => 1;

      let result =
        Tokenizer.tokenize(
          ~startIndex=5,
          ~endIndex=8,
          ~f=noSplit,
          ~measure,
          "abc" |> makeLine,
        );

      let runs = [];

      validateTokens(expect, result, runs);
    });
    test(
      "only part of token is produced when start / end index is specified",
      ({expect}) => {
      let measure = _ => 1;

      let result =
        Tokenizer.tokenize(
          ~startIndex=1,
          ~endIndex=3,
          ~f=noSplit,
          ~measure,
          "abcd" |> makeLine,
        );

      let runs = [
        TextRun.create(
          ~text="bc",
          ~startIndex=Index.fromZeroBased(1),
          ~endIndex=Index.fromZeroBased(3),
          ~startPosition=Index.fromZeroBased(1),
          ~endPosition=Index.fromZeroBased(3),
          (),
        ),
      ];

      validateTokens(expect, result, runs);
    });
    test("offset prior to tokenize is handled", ({expect}) => {
      // Pretend 'b' is actually 2 characters wide
      let measure = thickB;

      let result =
        Tokenizer.tokenize(
          ~startIndex=2,
          ~endIndex=4,
          ~f=noSplit,
          ~measure,
          "bbbcd" |> makeLine,
        );

      let runs = [
        TextRun.create(
          ~text="bc",
          ~startIndex=Index.fromZeroBased(2),
          ~endIndex=Index.fromZeroBased(4),
          ~startPosition=Index.fromZeroBased(4),
          ~endPosition=Index.fromZeroBased(7),
          (),
        ),
      ];

      validateTokens(expect, result, runs);
    });
  });

  describe("character measurement", ({test, _}) =>
    test("wide b", ({expect}) => {
      let str = "abab";
      let result =
        Tokenizer.tokenize(
          ~f=splitOnCharacter,
          ~measure=thickB,
          str |> makeLine,
        );

      let runs = [
        TextRun.create(
          ~text="a",
          ~startIndex=Index.zero,
          ~endIndex=Index.fromZeroBased(1),
          ~startPosition=Index.zero,
          ~endPosition=Index.fromZeroBased(1),
          (),
        ),
        TextRun.create(
          ~text="b",
          ~startIndex=Index.fromZeroBased(1),
          ~endIndex=Index.fromZeroBased(2),
          ~startPosition=Index.fromZeroBased(1),
          ~endPosition=Index.fromZeroBased(3),
          (),
        ),
        TextRun.create(
          ~text="a",
          ~startIndex=Index.fromZeroBased(2),
          ~endIndex=Index.fromZeroBased(3),
          ~startPosition=Index.fromZeroBased(3),
          ~endPosition=Index.fromZeroBased(4),
          (),
        ),
        TextRun.create(
          ~text="b",
          ~startIndex=Index.fromZeroBased(3),
          ~endIndex=Index.fromZeroBased(4),
          ~startPosition=Index.fromZeroBased(4),
          ~endPosition=Index.fromZeroBased(6),
          (),
        ),
      ];

      validateTokens(expect, result, runs);
    })
  );

  test("empty string", ({expect}) => {
    let result = Tokenizer.tokenize(~f=alwaysSplit, "" |> makeLine);
    expect.int(List.length(result)).toBe(0);
  });

  test("string broken up by characters", ({expect}) => {
    let str = "abab" |> makeLine;
    let result = Tokenizer.tokenize(~f=splitOnCharacter, str);

    let runs = [
      TextRun.create(
        ~text="a",
        ~startIndex=Index.zero,
        ~endIndex=Index.fromZeroBased(1),
        ~startPosition=Index.zero,
        ~endPosition=Index.fromZeroBased(1),
        (),
      ),
      TextRun.create(
        ~text="b",
        ~startIndex=Index.fromZeroBased(1),
        ~endIndex=Index.fromZeroBased(2),
        ~startPosition=Index.fromZeroBased(1),
        ~endPosition=Index.fromZeroBased(2),
        (),
      ),
      TextRun.create(
        ~text="a",
        ~startIndex=Index.fromZeroBased(2),
        ~endIndex=Index.fromZeroBased(3),
        ~startPosition=Index.fromZeroBased(2),
        ~endPosition=Index.fromZeroBased(3),
        (),
      ),
      TextRun.create(
        ~text="b",
        ~startIndex=Index.fromZeroBased(3),
        ~endIndex=Index.fromZeroBased(4),
        ~startPosition=Index.fromZeroBased(3),
        ~endPosition=Index.fromZeroBased(4),
        (),
      ),
    ];

    validateTokens(expect, result, runs);
  });

  test("string broken up by characters", ({expect}) => {
    let str = "aabbbbaa";
    let result = Tokenizer.tokenize(~f=splitOnCharacter, str |> makeLine);

    let runs = [
      TextRun.create(
        ~text="aa",
        ~startIndex=Index.zero,
        ~endIndex=Index.fromZeroBased(2),
        ~startPosition=Index.zero,
        ~endPosition=Index.fromZeroBased(2),
        (),
      ),
      TextRun.create(
        ~text="bbbb",
        ~startIndex=Index.fromZeroBased(2),
        ~endIndex=Index.fromZeroBased(6),
        ~startPosition=Index.fromZeroBased(2),
        ~endPosition=Index.fromZeroBased(6),
        (),
      ),
      TextRun.create(
        ~text="aa",
        ~startIndex=Index.fromZeroBased(6),
        ~endIndex=Index.fromZeroBased(8),
        ~startPosition=Index.fromZeroBased(6),
        ~endPosition=Index.fromZeroBased(8),
        (),
      ),
    ];

    validateTokens(expect, result, runs);
  });
});
