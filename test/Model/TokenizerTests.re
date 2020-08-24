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
  expect.int(actualToken.startByte |> ByteIndex.toInt).toBe(
    expectedToken.startByte |> ByteIndex.toInt,
  );
  expect.int(actualToken.endByte |> ByteIndex.toInt).toBe(
    expectedToken.endByte |> ByteIndex.toInt,
  );
  expect.int(actualToken.startIndex |> CharacterIndex.toInt).toBe(
    expectedToken.startIndex |> CharacterIndex.toInt,
  );
  expect.int(actualToken.endIndex |> CharacterIndex.toInt).toBe(
    CharacterIndex.toInt(expectedToken.endIndex),
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
          ~start=CharacterIndex.(zero + 1),
          ~stop=CharacterIndex.(zero + 3),
          ~f=noSplit,
          "" |> makeLine,
        );

      let runs = [];

      validateTokens(expect, result, runs);
    });
    test("start index past string", ({expect, _}) => {
      let result =
        Tokenizer.tokenize(
          ~start=CharacterIndex.(zero + 5),
          ~stop=CharacterIndex.(zero + 8),
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
          ~start=CharacterIndex.(zero + 1),
          ~stop=CharacterIndex.(zero + 3),
          ~f=noSplit,
          "abcd" |> makeLine,
        );

      let runs = [
        TextRun.create(
          ~text="bc",
          ~startByte=ByteIndex.ofInt(1),
          ~endByte=ByteIndex.ofInt(3),
          ~startIndex=CharacterIndex.ofInt(1),
          ~endIndex=CharacterIndex.ofInt(3),
          (),
        ),
      ];

      validateTokens(expect, result, runs);
    });
    test("offset prior to tokenize is handled", ({expect, _}) => {
      // Use '\t' which is two characters wide with default settings
      let result =
        Tokenizer.tokenize(
          ~start=3 |> CharacterIndex.ofInt,
          ~stop=5 |> CharacterIndex.ofInt,
          ~f=noSplit,
          "\t\t\tcd" |> makeLine,
        );

      let runs = [
        TextRun.create(
          ~text="cd",
          ~startByte=ByteIndex.ofInt(3),
          ~endByte=ByteIndex.ofInt(5),
          ~startIndex=CharacterIndex.ofInt(3),
          ~endIndex=CharacterIndex.ofInt(5),
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
        Tokenizer.tokenize(
          ~stop=CharacterIndex.ofInt(4),
          ~f=splitOnCharacter,
          str |> makeLine,
        );

      let runs = [
        TextRun.create(
          ~text="a",
          ~startByte=ByteIndex.ofInt(0),
          ~endByte=ByteIndex.ofInt(1),
          ~startIndex=CharacterIndex.zero,
          ~endIndex=CharacterIndex.ofInt(1),
          (),
        ),
        TextRun.create(
          ~text="\t",
          ~startByte=ByteIndex.ofInt(1),
          ~endByte=ByteIndex.ofInt(2),
          ~startIndex=CharacterIndex.ofInt(1),
          ~endIndex=CharacterIndex.ofInt(2),
          (),
        ),
        TextRun.create(
          ~text="a",
          ~startByte=ByteIndex.ofInt(2),
          ~endByte=ByteIndex.ofInt(3),
          ~startIndex=CharacterIndex.ofInt(2),
          ~endIndex=CharacterIndex.ofInt(3),
          (),
        ),
        TextRun.create(
          ~text="\t",
          ~startByte=ByteIndex.ofInt(3),
          ~endByte=ByteIndex.ofInt(4),
          ~startIndex=CharacterIndex.ofInt(3),
          ~endIndex=CharacterIndex.ofInt(4),
          (),
        ),
      ];

      validateTokens(expect, result, runs);
    })
  );

  test("empty string", ({expect, _}) => {
    let result =
      Tokenizer.tokenize(
        ~stop=CharacterIndex.zero,
        ~f=alwaysSplit,
        "" |> makeLine,
      );
    expect.int(List.length(result)).toBe(0);
  });

  test("string broken up by characters", ({expect, _}) => {
    let str = "abab" |> makeLine;
    let result =
      Tokenizer.tokenize(
        ~stop=CharacterIndex.ofInt(4),
        ~f=splitOnCharacter,
        str,
      );

    let runs = [
      TextRun.create(
        ~text="a",
        ~startByte=ByteIndex.ofInt(0),
        ~endByte=ByteIndex.ofInt(1),
        ~startIndex=CharacterIndex.zero,
        ~endIndex=CharacterIndex.ofInt(1),
        (),
      ),
      TextRun.create(
        ~text="b",
        ~startByte=ByteIndex.ofInt(1),
        ~endByte=ByteIndex.ofInt(2),
        ~startIndex=CharacterIndex.ofInt(1),
        ~endIndex=CharacterIndex.ofInt(2),
        (),
      ),
      TextRun.create(
        ~text="a",
        ~startByte=ByteIndex.ofInt(2),
        ~endByte=ByteIndex.ofInt(3),
        ~startIndex=CharacterIndex.ofInt(2),
        ~endIndex=CharacterIndex.ofInt(3),
        (),
      ),
      TextRun.create(
        ~text="b",
        ~startByte=ByteIndex.ofInt(3),
        ~endByte=ByteIndex.ofInt(4),
        ~startIndex=CharacterIndex.ofInt(3),
        ~endIndex=CharacterIndex.ofInt(4),
        (),
      ),
    ];

    validateTokens(expect, result, runs);
  });
  test("simple multi-byte case", ({expect, _}) => {
    let str = "κόσμε";
    let result =
      Tokenizer.tokenize(
        ~stop=String.length(str) |> CharacterIndex.ofInt,
        ~f=noSplit,
        str |> makeLine,
      );

    let runs = [
      TextRun.create(
        ~text="κόσμε",
        ~startByte=ByteIndex.ofInt(0),
        ~endByte=ByteIndex.ofInt(11),
        ~startIndex=CharacterIndex.zero,
        ~endIndex=CharacterIndex.ofInt(5),
        (),
      ),
    ];

    validateTokens(expect, result, runs);
  });

  test("string broken up by characters", ({expect, _}) => {
    let str = "aabbbbaa";
    let result =
      Tokenizer.tokenize(
        ~stop=String.length(str) |> CharacterIndex.ofInt,
        ~f=splitOnCharacter,
        str |> makeLine,
      );

    let runs = [
      TextRun.create(
        ~text="aa",
        ~startByte=ByteIndex.ofInt(0),
        ~endByte=ByteIndex.ofInt(2),
        ~startIndex=CharacterIndex.zero,
        ~endIndex=CharacterIndex.ofInt(2),
        (),
      ),
      TextRun.create(
        ~text="bbbb",
        ~startByte=ByteIndex.ofInt(2),
        ~endByte=ByteIndex.ofInt(6),
        ~startIndex=CharacterIndex.ofInt(2),
        ~endIndex=CharacterIndex.ofInt(6),
        (),
      ),
      TextRun.create(
        ~text="aa",
        ~startByte=ByteIndex.ofInt(6),
        ~endByte=ByteIndex.ofInt(8),
        ~startIndex=CharacterIndex.ofInt(6),
        ~endIndex=CharacterIndex.ofInt(8),
        (),
      ),
    ];

    validateTokens(expect, result, runs);
  });
});
