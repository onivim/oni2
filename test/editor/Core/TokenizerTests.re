open Revery;
open Oni_Core;
open TestFramework;

open Helpers;

let theme = Theme.create();

describe("tokenize", ({test, _}) => {
  test("empty string", ({expect}) => {
    let result = Tokenizer.tokenize("", theme);
    expect.int(List.length(result)).toBe(0);
  });

  test("string with only whitespace", ({expect}) => {
    let result = Tokenizer.tokenize("   \t", theme);
    expect.int(List.length(result)).toBe(0);
  });

  test("single word token", ({expect}) => {
    let result = Tokenizer.tokenize("testWord", theme);

    let expectedTokens: list(Tokenizer.t) = [
      {
        text: "testWord",
        startPosition: ZeroBasedIndex(0),
        endPosition: ZeroBasedIndex(8),
        color: Colors.red,
      },
    ];

    validateTokens(expect, result, expectedTokens);
  });

  test("single word token, surrounded by whitespace", ({expect}) => {
    let result = Tokenizer.tokenize("  testWord  ", theme);

    let expectedTokens: list(Tokenizer.t) = [
      {
        text: "testWord",
        startPosition: ZeroBasedIndex(2),
        endPosition: ZeroBasedIndex(10),
        color: Colors.red,
      },
    ];

    validateTokens(expect, result, expectedTokens);
  });

  test("single letter token, no spaces", ({expect}) => {
    let result = Tokenizer.tokenize("a", theme);

    let expectedTokens: list(Tokenizer.t) = [
      {
        text: "a",
        startPosition: ZeroBasedIndex(0),
        endPosition: ZeroBasedIndex(1),
        color: Colors.red,
      },
    ];

    validateTokens(expect, result, expectedTokens);
  });

  test("multiple tokens", ({expect}) => {
    let result = Tokenizer.tokenize(" a btest ", theme);

    let expectedTokens: list(Tokenizer.t) = [
      {
        text: "a",
        startPosition: ZeroBasedIndex(1),
        endPosition: ZeroBasedIndex(2),
        color: Colors.red,
      },
      {
        text: "btest",
        startPosition: ZeroBasedIndex(3),
        endPosition: ZeroBasedIndex(8),
        color: Colors.red,
      },
    ];

    validateTokens(expect, result, expectedTokens);
  });
});
