open Revery;
open Oni_Core;
open Oni_Model;
open Oni_Extensions;
open TestFramework;

open Helpers;

let theme = Theme.create();

let tokenColors = [];
let colorMap = ColorMap.create();

let indentation = IndentationSettings.default;

describe("BufferViewTokenizer", ({describe, test, _}) => {
  test("empty string", ({expect}) => {
    let result =
      BufferViewTokenizer.tokenize(
        "",
        theme,
        tokenColors,
        colorMap,
        indentation,
        None,
      );
    expect.int(List.length(result)).toBe(0);
  });

  describe("indentation settings", ({test, _}) =>
    test("accounts for tab size", ({expect}) => {
      let indentation =
        IndentationSettings.create(~mode=Tabs, ~size=2, ~tabSize=4, ());
      let result =
        BufferViewTokenizer.tokenize(
          "\tabc",
          theme,
          tokenColors,
          colorMap,
          indentation,
          None,
        );

      let expectedTokens: list(BufferViewTokenizer.t) = [
        {
          tokenType: Tab,
          text: "\t",
          startPosition: ZeroBasedIndex(0),
          endPosition: ZeroBasedIndex(4),
          color: Colors.red,
          backgroundColor: Colors.red,
        },
        {
          tokenType: Text,
          text: "abc",
          startPosition: ZeroBasedIndex(4),
          endPosition: ZeroBasedIndex(7),
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
        "   \t",
        theme,
        tokenColors,
        colorMap,
        indentation,
        None,
      );
    expect.int(List.length(result)).toBe(2);
  });

  test("single word token", ({expect}) => {
    let result =
      BufferViewTokenizer.tokenize(
        "testWord",
        theme,
        tokenColors,
        colorMap,
        indentation,
        None,
      );

    let expectedTokens: list(BufferViewTokenizer.t) = [
      {
        tokenType: Text,
        text: "testWord",
        startPosition: ZeroBasedIndex(0),
        endPosition: ZeroBasedIndex(8),
        color: Colors.red,
        backgroundColor: Colors.white,
      },
    ];

    validateTokens(expect, result, expectedTokens);
  });

  test("single word token, surrounded by whitespace", ({expect}) => {
    let result =
      BufferViewTokenizer.tokenize(
        "  testWord  ",
        theme,
        tokenColors,
        colorMap,
        indentation,
        None,
      );

    let expectedTokens: list(BufferViewTokenizer.t) = [
      {
        tokenType: Whitespace,
        text: "  ",
        startPosition: ZeroBasedIndex(0),
        endPosition: ZeroBasedIndex(2),
        color: Colors.red,
        backgroundColor: Colors.red,
      },
      {
        tokenType: Text,
        text: "testWord",
        startPosition: ZeroBasedIndex(2),
        endPosition: ZeroBasedIndex(10),
        color: Colors.red,
        backgroundColor: Colors.white,
      },
      {
        tokenType: Whitespace,
        text: "  ",
        startPosition: ZeroBasedIndex(10),
        endPosition: ZeroBasedIndex(12),
        color: Colors.red,
        backgroundColor: Colors.white,
      },
    ];

    validateTokens(expect, result, expectedTokens);
  });

  test("single letter token, no spaces", ({expect}) => {
    let result =
      BufferViewTokenizer.tokenize(
        "a",
        theme,
        tokenColors,
        colorMap,
        indentation,
        None,
      );

    let expectedTokens: list(BufferViewTokenizer.t) = [
      {
        tokenType: Text,
        text: "a",
        startPosition: ZeroBasedIndex(0),
        endPosition: ZeroBasedIndex(1),
        color: Colors.red,
        backgroundColor: Colors.white,
      },
    ];

    validateTokens(expect, result, expectedTokens);
  });

  test("respects tokenColor breaks", ({expect}) => {
    let tokenColors = [
      ColorizedToken.create(0, 0),
      ColorizedToken.create(1, 0),
    ];
    let result =
      BufferViewTokenizer.tokenize(
        "ab",
        theme,
        tokenColors,
        colorMap,
        indentation,
        None,
      );

    let expectedTokens: list(BufferViewTokenizer.t) = [
      {
        tokenType: Text,
        text: "a",
        startPosition: ZeroBasedIndex(0),
        endPosition: ZeroBasedIndex(1),
        color: Colors.red,
        backgroundColor: Colors.white,
      },
      {
        tokenType: Text,
        text: "b",
        startPosition: ZeroBasedIndex(1),
        endPosition: ZeroBasedIndex(2),
        color: Colors.red,
        backgroundColor: Colors.white,
      },
    ];

    validateTokens(expect, result, expectedTokens);
  });

  test("multiple tokens", ({expect}) => {
    let result =
      BufferViewTokenizer.tokenize(
        " a btest ",
        theme,
        tokenColors,
        colorMap,
        indentation,
        None,
      );

    let expectedTokens: list(BufferViewTokenizer.t) = [
      {
        tokenType: Whitespace,
        text: " ",
        startPosition: ZeroBasedIndex(0),
        endPosition: ZeroBasedIndex(1),
        color: Colors.red,
        backgroundColor: Colors.white,
      },
      {
        tokenType: Text,
        text: "a",
        startPosition: ZeroBasedIndex(1),
        endPosition: ZeroBasedIndex(2),
        color: Colors.red,
        backgroundColor: Colors.white,
      },
      {
        tokenType: Whitespace,
        text: " ",
        startPosition: ZeroBasedIndex(2),
        endPosition: ZeroBasedIndex(3),
        color: Colors.red,
        backgroundColor: Colors.white,
      },
      {
        tokenType: Text,
        text: "btest",
        startPosition: ZeroBasedIndex(3),
        endPosition: ZeroBasedIndex(8),
        color: Colors.red,
        backgroundColor: Colors.white,
      },
      {
        tokenType: Whitespace,
        text: " ",
        startPosition: ZeroBasedIndex(8),
        endPosition: ZeroBasedIndex(9),
        color: Colors.red,
        backgroundColor: Colors.white,
      },
    ];

    validateTokens(expect, result, expectedTokens);
  });
});
