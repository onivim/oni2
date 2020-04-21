open EditorCoreTypes;
open Oni_Core;
open TestFramework;

module TextRun = Tokenizer.TextRun;

let alwaysSplit =
    (
      ~index0 as _,
      ~byte0 as _,
      ~char0 as _,
      ~index1 as _,
      ~byte1 as _,
      ~char1 as _,
    ) =>
  true;
let noSplit =
    (
      ~index0 as _,
      ~byte0 as _,
      ~char0 as _,
      ~index1 as _,
      ~byte1 as _,
      ~char1 as _,
    ) =>
  false;
let splitOnCharacter =
    (~index0 as _, ~byte0 as _, ~char0, ~index1 as _, ~byte1 as _, ~char1) =>
  !Uchar.equal(char0, char1);

let thickB = c =>
  switch (Uchar.to_char(c)) {
  | 'b' => 2
  | _ => 1
  };

let makeLine =
  BufferLine.make(
    ~indentation=
      IndentationSettings.create(~mode=Tabs, ~size=2, ~tabSize=2, ()),
  );

let validateToken =
    (
      expect: Rely.matchers(unit),
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
      expect: Rely.matchers(unit),
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
    test("empty string returns nothing", ({expect, _}) => {
      let result =
        Tokenizer.tokenize(
          ~startIndex=1,
          ~endIndex=3,
          ~f=noSplit,
          "" |> makeLine,
        );

      let runs = [];

      validateTokens(expect, result, runs);
    });
    test("start index past string", ({expect, _}) => {
      let result =
        Tokenizer.tokenize(
          ~startIndex=5,
          ~endIndex=8,
          ~f=noSplit,
          "abc" |> makeLine,
        );

      let runs = [];

      validateTokens(expect, result, runs);
    });
    test(
      "only part of token is produced when start / end index is specified",
      ({expect, _}) => {
      let result =
        Tokenizer.tokenize(
          ~startIndex=1,
          ~endIndex=3,
          ~f=noSplit,
          "abcd" |> makeLine,
        );

      let runs = [
        TextRun.create(
          ~text="bc",
          ~startByte=1,
          ~endByte=3,
          ~startIndex=Index.fromZeroBased(1),
          ~endIndex=Index.fromZeroBased(3),
          ~startPosition=Index.fromZeroBased(1),
          ~endPosition=Index.fromZeroBased(3),
          (),
        ),
      ];

      validateTokens(expect, result, runs);
    });
    test("offset prior to tokenize is handled", ({expect, _}) => {
      // Use '\t' which is two characters wide with default settings
      let result =
        Tokenizer.tokenize(
          ~startIndex=3,
          ~endIndex=5,
          ~f=noSplit,
          "\t\t\tcd" |> makeLine,
        );

      let runs = [
        TextRun.create(
          ~text="cd",
          ~startByte=3,
          ~endByte=5,
          ~startIndex=Index.fromZeroBased(3),
          ~endIndex=Index.fromZeroBased(5),
          ~startPosition=Index.fromZeroBased(6),
          ~endPosition=Index.fromZeroBased(8),
          (),
        ),
      ];

      validateTokens(expect, result, runs);
    });
  });

  describe("character measurement", ({test, _}) =>
    test("wide tab", ({expect, _}) => {
      let str = "a\ta\t";
      let result =
        Tokenizer.tokenize(~endIndex=4, ~f=splitOnCharacter, str |> makeLine);

      let runs = [
        TextRun.create(
          ~text="a",
          ~startByte=0,
          ~endByte=1,
          ~startIndex=Index.zero,
          ~endIndex=Index.fromZeroBased(1),
          ~startPosition=Index.zero,
          ~endPosition=Index.fromZeroBased(1),
          (),
        ),
        TextRun.create(
          ~text="\t",
          ~startByte=1,
          ~endByte=2,
          ~startIndex=Index.fromZeroBased(1),
          ~endIndex=Index.fromZeroBased(2),
          ~startPosition=Index.fromZeroBased(1),
          ~endPosition=Index.fromZeroBased(3),
          (),
        ),
        TextRun.create(
          ~text="a",
          ~startByte=2,
          ~endByte=3,
          ~startIndex=Index.fromZeroBased(2),
          ~endIndex=Index.fromZeroBased(3),
          ~startPosition=Index.fromZeroBased(3),
          ~endPosition=Index.fromZeroBased(4),
          (),
        ),
        TextRun.create(
          ~text="\t",
          ~startByte=3,
          ~endByte=4,
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

  test("empty string", ({expect, _}) => {
    let result =
      Tokenizer.tokenize(~endIndex=0, ~f=alwaysSplit, "" |> makeLine);
    expect.int(List.length(result)).toBe(0);
  });

  test("string broken up by characters", ({expect, _}) => {
    let str = "abab" |> makeLine;
    let result = Tokenizer.tokenize(~endIndex=4, ~f=splitOnCharacter, str);

    let runs = [
      TextRun.create(
        ~text="a",
        ~startByte=0,
        ~endByte=1,
        ~startIndex=Index.zero,
        ~endIndex=Index.fromZeroBased(1),
        ~startPosition=Index.zero,
        ~endPosition=Index.fromZeroBased(1),
        (),
      ),
      TextRun.create(
        ~text="b",
        ~startByte=1,
        ~endByte=2,
        ~startIndex=Index.fromZeroBased(1),
        ~endIndex=Index.fromZeroBased(2),
        ~startPosition=Index.fromZeroBased(1),
        ~endPosition=Index.fromZeroBased(2),
        (),
      ),
      TextRun.create(
        ~text="a",
        ~startByte=2,
        ~endByte=3,
        ~startIndex=Index.fromZeroBased(2),
        ~endIndex=Index.fromZeroBased(3),
        ~startPosition=Index.fromZeroBased(2),
        ~endPosition=Index.fromZeroBased(3),
        (),
      ),
      TextRun.create(
        ~text="b",
        ~startByte=3,
        ~endByte=4,
        ~startIndex=Index.fromZeroBased(3),
        ~endIndex=Index.fromZeroBased(4),
        ~startPosition=Index.fromZeroBased(3),
        ~endPosition=Index.fromZeroBased(4),
        (),
      ),
    ];

    validateTokens(expect, result, runs);
  });

  test("string broken up by characters", ({expect, _}) => {
    let str = "aabbbbaa";
    let result =
      Tokenizer.tokenize(
        ~endIndex=String.length(str),
        ~f=splitOnCharacter,
        str |> makeLine,
      );

    let runs = [
      TextRun.create(
        ~text="aa",
        ~startByte=0,
        ~endByte=2,
        ~startIndex=Index.zero,
        ~endIndex=Index.fromZeroBased(2),
        ~startPosition=Index.zero,
        ~endPosition=Index.fromZeroBased(2),
        (),
      ),
      TextRun.create(
        ~text="bbbb",
        ~startByte=0,
        ~endByte=0,
        ~startIndex=Index.fromZeroBased(2),
        ~endIndex=Index.fromZeroBased(6),
        ~startPosition=Index.fromZeroBased(2),
        ~endPosition=Index.fromZeroBased(6),
        (),
      ),
      TextRun.create(
        ~text="aa",
        ~startByte=6,
        ~endByte=8,
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
