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

let splitColorizer = (split, idx) =>
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

let makeLine = str => BufferLine.make(~indentation, str);

describe("BufferViewTokenizer", ({describe, test, _}) => {
  test("empty string", ({expect, _}) => {
    let result =
      BufferViewTokenizer.tokenize(
        ~endIndex=0,
        "" |> makeLine,
        basicColorizer,
      );
    expect.int(List.length(result)).toBe(0);
  });

  test("multi-byte case", ({expect, _}) => {
    let result =
      BufferViewTokenizer.tokenize(
        ~endIndex=9,
        "κόσμε abc" |> BufferLine.make(~indentation),
        // Split at byte 11 - after the multi-byte characters
        splitColorizer(11),
      );

    let expectedTokens: list(BufferViewTokenizer.t) = [
      {
        tokenType: Text,
        text: "κόσμε",
        startIndex: Index.zero,
        endIndex: Index.fromZeroBased(5),
        color: Colors.red,
        backgroundColor: Colors.red,
        bold: false,
        italic: false,
      },
      {
        tokenType: Whitespace,
        text: " ",
        startIndex: Index.fromZeroBased(5),
        endIndex: Index.fromZeroBased(6),
        color: Colors.green,
        backgroundColor: Colors.green,
        bold: false,
        italic: false,
      },
      {
        tokenType: Text,
        text: "abc",
        startIndex: Index.fromZeroBased(6),
        endIndex: Index.fromZeroBased(9),
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
          ~endIndex=4,
          "\tabc" |> BufferLine.make(~indentation),
          basicColorizer,
        );

      let expectedTokens: list(BufferViewTokenizer.t) = [
        {
          tokenType: Tab,
          text: "\t",
          startIndex: Index.zero,
          endIndex: Index.fromZeroBased(4),
          color: Colors.red,
          backgroundColor: Colors.red,
          bold: false,
          italic: false,
        },
        {
          tokenType: Text,
          text: "abc",
          startIndex: Index.fromZeroBased(4),
          endIndex: Index.fromZeroBased(7),
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
        ~endIndex=4,
        "   \t" |> makeLine,
        basicColorizer,
      );
    expect.int(List.length(result)).toBe(2);
  });

  test("single word token", ({expect, _}) => {
    let result =
      BufferViewTokenizer.tokenize(
        ~endIndex=8,
        "testWord" |> makeLine,
        basicColorizer,
      );

    let expectedTokens: list(BufferViewTokenizer.t) = [
      {
        tokenType: Text,
        text: "testWord",
        startIndex: Index.zero,
        endIndex: Index.fromZeroBased(8),
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
        ~endIndex=12,
        "  testWord  " |> makeLine,
        basicColorizer,
      );

    let expectedTokens: list(BufferViewTokenizer.t) = [
      {
        tokenType: Whitespace,
        text: "  ",
        startIndex: Index.zero,
        endIndex: Index.fromZeroBased(2),
        color: Colors.red,
        backgroundColor: Colors.red,
        bold: false,
        italic: false,
      },
      {
        tokenType: Text,
        text: "testWord",
        startIndex: Index.fromZeroBased(2),
        endIndex: Index.fromZeroBased(10),
        color: Colors.red,
        backgroundColor: Colors.white,
        bold: false,
        italic: false,
      },
      {
        tokenType: Whitespace,
        text: "  ",
        startIndex: Index.fromZeroBased(10),
        endIndex: Index.fromZeroBased(12),
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
        ~endIndex=1,
        "a" |> makeLine,
        basicColorizer,
      );

    let expectedTokens: list(BufferViewTokenizer.t) = [
      {
        tokenType: Text,
        text: "a",
        startIndex: Index.zero,
        endIndex: Index.fromZeroBased(1),
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
      i > 0
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
        ~endIndex=2,
        "ab" |> makeLine,
        differentColorTokenizer,
      );

    let expectedTokens: list(BufferViewTokenizer.t) = [
      {
        tokenType: Text,
        text: "a",
        startIndex: Index.zero,
        endIndex: Index.fromZeroBased(1),
        color: Colors.red,
        backgroundColor: Colors.white,
        bold: false,
        italic: false,
      },
      {
        tokenType: Text,
        text: "b",
        startIndex: Index.fromZeroBased(1),
        endIndex: Index.fromZeroBased(2),
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
        ~endIndex=9,
        " a btest " |> makeLine,
        basicColorizer,
      );

    let expectedTokens: list(BufferViewTokenizer.t) = [
      {
        tokenType: Whitespace,
        text: " ",
        startIndex: Index.zero,
        endIndex: Index.fromZeroBased(1),
        color: Colors.red,
        backgroundColor: Colors.white,
        bold: false,
        italic: false,
      },
      {
        tokenType: Text,
        text: "a",
        startIndex: Index.fromZeroBased(1),
        endIndex: Index.fromZeroBased(2),
        color: Colors.red,
        backgroundColor: Colors.white,
        bold: false,
        italic: false,
      },
      {
        tokenType: Whitespace,
        text: " ",
        startIndex: Index.fromZeroBased(2),
        endIndex: Index.fromZeroBased(3),
        color: Colors.red,
        backgroundColor: Colors.white,
        bold: false,
        italic: false,
      },
      {
        tokenType: Text,
        text: "btest",
        startIndex: Index.fromZeroBased(3),
        endIndex: Index.fromZeroBased(8),
        color: Colors.red,
        backgroundColor: Colors.white,
        bold: false,
        italic: false,
      },
      {
        tokenType: Whitespace,
        text: " ",
        startIndex: Index.fromZeroBased(8),
        endIndex: Index.fromZeroBased(9),
        color: Colors.red,
        backgroundColor: Colors.white,
        bold: false,
        italic: false,
      },
    ];

    validateTokens(expect, result, expectedTokens);
  });
});
