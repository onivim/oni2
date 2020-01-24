open EditorCoreTypes;
open Revery;
open Oni_Core;
open Oni_Model;
open TestFramework;

open Helpers;

let theme = Theme.default;

let indentation = IndentationSettings.default;

let basicColorizer = _ => (Colors.black, Colors.white);

let makeLine = str => BufferLine.make(~indentation, str);

describe("BufferViewTokenizer", ({describe, test, _}) => {
  test("empty string", ({expect}) => {
    let result =
      BufferViewTokenizer.tokenize(
        ~endIndex=0,
        "" |> makeLine,
        indentation,
        basicColorizer,
      );
    expect.int(List.length(result)).toBe(0);
  });

  describe("indentation settings", ({test, _}) =>
    test("accounts for tab size", ({expect}) => {
      let indentation =
        IndentationSettings.create(~mode=Tabs, ~size=2, ~tabSize=4, ());
      let result =
        BufferViewTokenizer.tokenize(
          ~endIndex=4,
          "\tabc" |> makeLine,
          indentation,
          basicColorizer,
        );

      let expectedTokens: list(BufferViewTokenizer.t) = [
        {
          tokenType: Tab,
          text: "\t",
          startPosition: Index.zero,
          endPosition: Index.fromZeroBased(4),
          color: Colors.red,
          backgroundColor: Colors.red,
        },
        {
          tokenType: Text,
          text: "abc",
          startPosition: Index.fromZeroBased(4),
          endPosition: Index.fromZeroBased(7),
          color: Colors.red,
          backgroundColor: Colors.white,
        },
      ];

      validateTokens(expect, result, expectedTokens);
    })
  );

  test("string with only whitespace", ({expect}) => {
    let result =
      BufferViewTokenizer.tokenize(
        ~endIndex=4,
        "   \t" |> makeLine,
        indentation,
        basicColorizer,
      );
    expect.int(List.length(result)).toBe(2);
  });

  test("single word token", ({expect}) => {
    let result =
      BufferViewTokenizer.tokenize(
        ~endIndex=8,
        "testWord" |> makeLine,
        indentation,
        basicColorizer,
      );

    let expectedTokens: list(BufferViewTokenizer.t) = [
      {
        tokenType: Text,
        text: "testWord",
        startPosition: Index.zero,
        endPosition: Index.fromZeroBased(8),
        color: Colors.red,
        backgroundColor: Colors.white,
      },
    ];

    validateTokens(expect, result, expectedTokens);
  });

  test("single word token, surrounded by whitespace", ({expect}) => {
    let result =
      BufferViewTokenizer.tokenize(
        ~endIndex=12,
        "  testWord  " |> makeLine,
        indentation,
        basicColorizer,
      );

    let expectedTokens: list(BufferViewTokenizer.t) = [
      {
        tokenType: Whitespace,
        text: "  ",
        startPosition: Index.zero,
        endPosition: Index.fromZeroBased(2),
        color: Colors.red,
        backgroundColor: Colors.red,
      },
      {
        tokenType: Text,
        text: "testWord",
        startPosition: Index.fromZeroBased(2),
        endPosition: Index.fromZeroBased(10),
        color: Colors.red,
        backgroundColor: Colors.white,
      },
      {
        tokenType: Whitespace,
        text: "  ",
        startPosition: Index.fromZeroBased(10),
        endPosition: Index.fromZeroBased(12),
        color: Colors.red,
        backgroundColor: Colors.white,
      },
    ];

    validateTokens(expect, result, expectedTokens);
  });

  test("single letter token, no spaces", ({expect}) => {
    let result =
      BufferViewTokenizer.tokenize(
        ~endIndex=1,
        "a" |> makeLine,
        indentation,
        basicColorizer,
      );

    let expectedTokens: list(BufferViewTokenizer.t) = [
      {
        tokenType: Text,
        text: "a",
        startPosition: Index.zero,
        endPosition: Index.fromZeroBased(1),
        color: Colors.red,
        backgroundColor: Colors.white,
      },
    ];

    validateTokens(expect, result, expectedTokens);
  });

  test("respects tokenColor breaks", ({expect}) => {
    let differentColorTokenizer = i =>
      i > 0 ? (Colors.green, Colors.yellow) : (Colors.black, Colors.white);

    let result =
      BufferViewTokenizer.tokenize(
        ~endIndex=2,
        "ab" |> makeLine,
        indentation,
        differentColorTokenizer,
      );

    let expectedTokens: list(BufferViewTokenizer.t) = [
      {
        tokenType: Text,
        text: "a",
        startPosition: Index.zero,
        endPosition: Index.fromZeroBased(1),
        color: Colors.red,
        backgroundColor: Colors.white,
      },
      {
        tokenType: Text,
        text: "b",
        startPosition: Index.fromZeroBased(1),
        endPosition: Index.fromZeroBased(2),
        color: Colors.red,
        backgroundColor: Colors.white,
      },
    ];

    validateTokens(expect, result, expectedTokens);
  });

  test("multiple tokens", ({expect}) => {
    let result =
      BufferViewTokenizer.tokenize(
        ~endIndex=9,
        " a btest " |> makeLine,
        indentation,
        basicColorizer,
      );

    let expectedTokens: list(BufferViewTokenizer.t) = [
      {
        tokenType: Whitespace,
        text: " ",
        startPosition: Index.zero,
        endPosition: Index.fromZeroBased(1),
        color: Colors.red,
        backgroundColor: Colors.white,
      },
      {
        tokenType: Text,
        text: "a",
        startPosition: Index.fromZeroBased(1),
        endPosition: Index.fromZeroBased(2),
        color: Colors.red,
        backgroundColor: Colors.white,
      },
      {
        tokenType: Whitespace,
        text: " ",
        startPosition: Index.fromZeroBased(2),
        endPosition: Index.fromZeroBased(3),
        color: Colors.red,
        backgroundColor: Colors.white,
      },
      {
        tokenType: Text,
        text: "btest",
        startPosition: Index.fromZeroBased(3),
        endPosition: Index.fromZeroBased(8),
        color: Colors.red,
        backgroundColor: Colors.white,
      },
      {
        tokenType: Whitespace,
        text: " ",
        startPosition: Index.fromZeroBased(8),
        endPosition: Index.fromZeroBased(9),
        color: Colors.red,
        backgroundColor: Colors.white,
      },
    ];

    validateTokens(expect, result, expectedTokens);
  });
});
