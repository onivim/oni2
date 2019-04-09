open Oni_Core.Types;
open Oni_Model;
open TestFramework;

open CamomileLibrary;

module TextRun = Tokenizer.TextRun;

let alwaysSplit = (_, _, _, _) => true;
let splitOnCharacter = (_, c1, _, c2) => !UChar.eq(c1, c2);

let validateToken =
    (
      expect: Rely__DefaultMatchers.matchers(unit),
      actualToken: TextRun.t,
      expectedToken: TextRun.t,
    ) => {
  expect.string(actualToken.text).toEqual(expectedToken.text);
  expect.int(Index.toZeroBasedInt(actualToken.startIndex)).toBe(
    Index.toZeroBasedInt(expectedToken.startIndex),
  );
  expect.int(Index.toZeroBasedInt(actualToken.endIndex)).toBe(
    Index.toZeroBasedInt(expectedToken.endIndex),
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

describe("tokenize", ({test, describe, _}) => {
  describe("character measurement", ({test, _}) =>
    test("wide b", ({expect}) => {
      let str = "abab";
      let thickB = c =>
        switch (UChar.char_of(c)) {
        | 'b' => 2
        | _ => 1
        };
      let result =
        Tokenizer.tokenize(~f=splitOnCharacter, ~measure=thickB, str);

      let runs = [
        TextRun.create(
          ~text="a",
          ~startIndex=ZeroBasedIndex(0),
          ~endIndex=ZeroBasedIndex(1),
          ~startPosition=ZeroBasedIndex(0),
          ~endPosition=ZeroBasedIndex(1),
          (),
        ),
        TextRun.create(
          ~text="b",
          ~startIndex=ZeroBasedIndex(1),
          ~endIndex=ZeroBasedIndex(2),
          ~startPosition=ZeroBasedIndex(1),
          ~endPosition=ZeroBasedIndex(3),
          (),
        ),
        TextRun.create(
          ~text="a",
          ~startIndex=ZeroBasedIndex(2),
          ~endIndex=ZeroBasedIndex(3),
          ~startPosition=ZeroBasedIndex(3),
          ~endPosition=ZeroBasedIndex(4),
          (),
        ),
        TextRun.create(
          ~text="b",
          ~startIndex=ZeroBasedIndex(3),
          ~endIndex=ZeroBasedIndex(4),
          ~startPosition=ZeroBasedIndex(4),
          ~endPosition=ZeroBasedIndex(6),
          (),
        ),
      ];

      validateTokens(expect, result, runs);
    });
  );

  test("empty string", ({expect}) => {
    let result = Tokenizer.tokenize(~f=alwaysSplit, "");
    expect.int(List.length(result)).toBe(0);
  });

  test("string broken up by characters", ({expect}) => {
    let str = "abab";
    let result = Tokenizer.tokenize(~f=splitOnCharacter, str);

    let runs = [
      TextRun.create(
        ~text="a",
        ~startIndex=ZeroBasedIndex(0),
        ~endIndex=ZeroBasedIndex(1),
        ~startPosition=ZeroBasedIndex(0),
        ~endPosition=ZeroBasedIndex(1),
        (),
      ),
      TextRun.create(
        ~text="b",
        ~startIndex=ZeroBasedIndex(1),
        ~endIndex=ZeroBasedIndex(2),
        ~startPosition=ZeroBasedIndex(1),
        ~endPosition=ZeroBasedIndex(2),
        (),
      ),
      TextRun.create(
        ~text="a",
        ~startIndex=ZeroBasedIndex(2),
        ~endIndex=ZeroBasedIndex(3),
        ~startPosition=ZeroBasedIndex(2),
        ~endPosition=ZeroBasedIndex(3),
        (),
      ),
      TextRun.create(
        ~text="b",
        ~startIndex=ZeroBasedIndex(3),
        ~endIndex=ZeroBasedIndex(4),
        ~startPosition=ZeroBasedIndex(3),
        ~endPosition=ZeroBasedIndex(4),
        (),
      ),
    ];

    validateTokens(expect, result, runs);
  });

  test("string broken up by characters", ({expect}) => {
    let str = "aabbbbaa";
    let result = Tokenizer.tokenize(~f=splitOnCharacter, str);

    let runs = [
      TextRun.create(
        ~text="aa",
        ~startIndex=ZeroBasedIndex(0),
        ~endIndex=ZeroBasedIndex(2),
        ~startPosition=ZeroBasedIndex(0),
        ~endPosition=ZeroBasedIndex(2),
        (),
      ),
      TextRun.create(
        ~text="bbbb",
        ~startIndex=ZeroBasedIndex(2),
        ~endIndex=ZeroBasedIndex(6),
        ~startPosition=ZeroBasedIndex(2),
        ~endPosition=ZeroBasedIndex(6),
        (),
      ),
      TextRun.create(
        ~text="aa",
        ~startIndex=ZeroBasedIndex(6),
        ~endIndex=ZeroBasedIndex(8),
        ~startPosition=ZeroBasedIndex(6),
        ~endPosition=ZeroBasedIndex(8),
        (),
      ),
    ];

    validateTokens(expect, result, runs);
  });
});
