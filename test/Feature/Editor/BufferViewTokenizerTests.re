open EditorCoreTypes;
open Revery;
open Oni_Core;
open TestFramework;

open Helpers;

module BufferViewTokenizer = Feature_Editor.BufferViewTokenizer;
module BufferLineColorizer = Feature_Editor.BufferLineColorizer;

let indentation = IndentationSettings.default;

let basicColorizer = _ =>
  BufferLineColorizer.{
    color: Colors.black,
    backgroundColor: Colors.white,
    bold: false,
    italic: false,
  };

let splitColorizer = (split, idx) => {
  let idx = ByteIndex.toInt(idx);
  if (idx < split) {
    BufferLineColorizer.{
      color: Colors.red,
      backgroundColor: Colors.red,
      bold: false,
      italic: false,
    };
  } else {
    BufferLineColorizer.{
      color: Colors.green,
      backgroundColor: Colors.green,
      bold: false,
      italic: false,
    };
  };
};

let makeLine = str => BufferLine.make(~indentation, str);

describe("BufferViewTokenizer", ({describe, test, _}) => {
  test("empty string", ({expect, _}) => {
    let result =
      BufferViewTokenizer.tokenize(
        ~stop=CharacterIndex.zero,
        "" |> makeLine,
        basicColorizer,
      );
    expect.int(List.length(result)).toBe(0);
  });

  test("multi-byte case", ({expect, _}) => {
    let result =
      BufferViewTokenizer.tokenize(
        ~stop=CharacterIndex.ofInt(9),
        "κόσμε abc" |> BufferLine.make(~indentation),
        // Split at byte 11 - after the multi-byte characters
        splitColorizer(11),
      );

    let expectedTokens: list(BufferViewTokenizer.t) = [
      {
        tokenType: Text,
        text: "κόσμε",
        startIndex: CharacterIndex.zero,
        endIndex: CharacterIndex.ofInt(5),
        startByte: ByteIndex.zero,
        endByte: ByteIndex.ofInt(13),
        color: Colors.red,
        backgroundColor: Colors.red,
        bold: false,
        italic: false,
      },
      {
        tokenType: Whitespace,
        text: " ",
        startIndex: CharacterIndex.ofInt(5),
        endIndex: CharacterIndex.ofInt(6),
        startByte: ByteIndex.ofInt(13),
        endByte: ByteIndex.ofInt(14),
        color: Colors.green,
        backgroundColor: Colors.green,
        bold: false,
        italic: false,
      },
      {
        tokenType: Text,
        text: "abc",
        startIndex: CharacterIndex.ofInt(6),
        endIndex: CharacterIndex.ofInt(9),
        startByte: ByteIndex.ofInt(14),
        endByte: ByteIndex.ofInt(17),
        color: Colors.green,
        backgroundColor: Colors.green,
        bold: false,
        italic: false,
      },
    ];
    validateTokens(expect, result, expectedTokens);
  });

  describe("indentation settings", ({test, _}) =>
    test("accounts for tab size", ({expect, _}) => {
      let indentation =
        IndentationSettings.create(~mode=Tabs, ~size=2, ~tabSize=4, ());
      let result =
        BufferViewTokenizer.tokenize(
          ~stop=CharacterIndex.ofInt(4),
          "\tabc" |> BufferLine.make(~indentation),
          basicColorizer,
        );

      let expectedTokens: list(BufferViewTokenizer.t) = [
        {
          tokenType: Tab,
          text: "\t",
          startIndex: CharacterIndex.zero,
          endIndex: CharacterIndex.ofInt(4),
          startByte: ByteIndex.ofInt(0),
          endByte: ByteIndex.ofInt(4),
          color: Colors.red,
          backgroundColor: Colors.red,
          bold: false,
          italic: false,
        },
        {
          tokenType: Text,
          text: "abc",
          startIndex: CharacterIndex.ofInt(4),
          endIndex: CharacterIndex.ofInt(7),
          startByte: ByteIndex.ofInt(4),
          endByte: ByteIndex.ofInt(7),
          color: Colors.red,
          backgroundColor: Colors.white,
          bold: false,
          italic: false,
        },
      ];

      validateTokens(expect, result, expectedTokens);
    })
  );

  test("string with only whitespace", ({expect, _}) => {
    let result =
      BufferViewTokenizer.tokenize(
        ~stop=CharacterIndex.ofInt(4),
        "   \t" |> makeLine,
        basicColorizer,
      );
    expect.int(List.length(result)).toBe(2);
  });

  test("single word token", ({expect, _}) => {
    let result =
      BufferViewTokenizer.tokenize(
        ~stop=CharacterIndex.ofInt(8),
        "testWord" |> makeLine,
        basicColorizer,
      );

    let expectedTokens: list(BufferViewTokenizer.t) = [
      {
        tokenType: Text,
        text: "testWord",
        startIndex: CharacterIndex.zero,
        endIndex: CharacterIndex.ofInt(8),
        startByte: ByteIndex.ofInt(0),
        endByte: ByteIndex.ofInt(8),
        color: Colors.red,
        backgroundColor: Colors.white,
        bold: false,
        italic: false,
      },
    ];

    validateTokens(expect, result, expectedTokens);
  });

  test("single word token, surrounded by whitespace", ({expect, _}) => {
    let result =
      BufferViewTokenizer.tokenize(
        ~stop=CharacterIndex.ofInt(12),
        "  testWord  " |> makeLine,
        basicColorizer,
      );

    let expectedTokens: list(BufferViewTokenizer.t) = [
      {
        tokenType: Whitespace,
        text: "  ",
        startIndex: CharacterIndex.zero,
        endIndex: CharacterIndex.ofInt(2),
        startByte: ByteIndex.ofInt(0),
        endByte: ByteIndex.ofInt(2),
        color: Colors.red,
        backgroundColor: Colors.red,
        bold: false,
        italic: false,
      },
      {
        tokenType: Text,
        text: "testWord",
        startIndex: CharacterIndex.ofInt(2),
        endIndex: CharacterIndex.ofInt(10),
        startByte: ByteIndex.ofInt(2),
        endByte: ByteIndex.ofInt(10),
        color: Colors.red,
        backgroundColor: Colors.white,
        bold: false,
        italic: false,
      },
      {
        tokenType: Whitespace,
        text: "  ",
        startIndex: CharacterIndex.ofInt(10),
        endIndex: CharacterIndex.ofInt(12),
        startByte: ByteIndex.ofInt(10),
        endByte: ByteIndex.ofInt(12),
        color: Colors.red,
        backgroundColor: Colors.white,
        bold: false,
        italic: false,
      },
    ];

    validateTokens(expect, result, expectedTokens);
  });

  test("single letter token, no spaces", ({expect, _}) => {
    let result =
      BufferViewTokenizer.tokenize(
        ~stop=CharacterIndex.ofInt(1),
        "a" |> makeLine,
        basicColorizer,
      );

    let expectedTokens: list(BufferViewTokenizer.t) = [
      {
        tokenType: Text,
        text: "a",
        startIndex: CharacterIndex.zero,
        endIndex: CharacterIndex.ofInt(1),
        startByte: ByteIndex.ofInt(0),
        endByte: ByteIndex.ofInt(1),
        color: Colors.red,
        backgroundColor: Colors.white,
        bold: false,
        italic: false,
      },
    ];

    validateTokens(expect, result, expectedTokens);
  });

  test("respects tokenColor breaks", ({expect, _}) => {
    let differentColorTokenizer = i =>
      ByteIndex.toInt(i) > 0
        ? BufferLineColorizer.{
            color: Colors.green,
            backgroundColor: Colors.yellow,
            bold: false,
            italic: false,
          }
        : BufferLineColorizer.{
            color: Colors.black,
            backgroundColor: Colors.white,
            bold: false,
            italic: false,
          };

    let result =
      BufferViewTokenizer.tokenize(
        ~stop=CharacterIndex.ofInt(2),
        "ab" |> makeLine,
        differentColorTokenizer,
      );

    let expectedTokens: list(BufferViewTokenizer.t) = [
      {
        tokenType: Text,
        text: "a",
        startIndex: CharacterIndex.zero,
        endIndex: CharacterIndex.ofInt(1),
        startByte: ByteIndex.ofInt(0),
        endByte: ByteIndex.ofInt(1),
        color: Colors.red,
        backgroundColor: Colors.white,
        bold: false,
        italic: false,
      },
      {
        tokenType: Text,
        text: "b",
        startIndex: CharacterIndex.ofInt(1),
        endIndex: CharacterIndex.ofInt(2),
        startByte: ByteIndex.ofInt(1),
        endByte: ByteIndex.ofInt(2),
        color: Colors.red,
        backgroundColor: Colors.white,
        bold: false,
        italic: false,
      },
    ];

    validateTokens(expect, result, expectedTokens);
  });

  test("multiple tokens", ({expect, _}) => {
    let result =
      BufferViewTokenizer.tokenize(
        ~stop=CharacterIndex.ofInt(9),
        " a btest " |> makeLine,
        basicColorizer,
      );

    let expectedTokens: list(BufferViewTokenizer.t) = [
      {
        tokenType: Whitespace,
        text: " ",
        startIndex: CharacterIndex.zero,
        endIndex: CharacterIndex.ofInt(1),
        startByte: ByteIndex.ofInt(0),
        endByte: ByteIndex.ofInt(1),
        color: Colors.red,
        backgroundColor: Colors.white,
        bold: false,
        italic: false,
      },
      {
        tokenType: Text,
        text: "a",
        startIndex: CharacterIndex.ofInt(1),
        endIndex: CharacterIndex.ofInt(2),
        startByte: ByteIndex.ofInt(1),
        endByte: ByteIndex.ofInt(2),
        color: Colors.red,
        backgroundColor: Colors.white,
        bold: false,
        italic: false,
      },
      {
        tokenType: Whitespace,
        text: " ",
        startIndex: CharacterIndex.ofInt(2),
        endIndex: CharacterIndex.ofInt(3),
        startByte: ByteIndex.ofInt(2),
        endByte: ByteIndex.ofInt(3),
        color: Colors.red,
        backgroundColor: Colors.white,
        bold: false,
        italic: false,
      },
      {
        tokenType: Text,
        text: "btest",
        startIndex: CharacterIndex.ofInt(3),
        endIndex: CharacterIndex.ofInt(8),
        startByte: ByteIndex.ofInt(3),
        endByte: ByteIndex.ofInt(8),
        color: Colors.red,
        backgroundColor: Colors.white,
        bold: false,
        italic: false,
      },
      {
        tokenType: Whitespace,
        text: " ",
        startIndex: CharacterIndex.ofInt(8),
        endIndex: CharacterIndex.ofInt(9),
        startByte: ByteIndex.ofInt(8),
        endByte: ByteIndex.ofInt(9),
        color: Colors.red,
        backgroundColor: Colors.white,
        bold: false,
        italic: false,
      },
    ];

    validateTokens(expect, result, expectedTokens);
  });
});
