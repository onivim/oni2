open Oni_Core;
open TestFramework;

open Helpers;

describe("tokenize", ({test, _}) => {
  test("empty string", ({expect}) => {
    let result = Tokenizer.tokenize("");
    expect.int(List.length(result)).toBe(0);
  });

  test("string with only whitespace", ({expect}) => {
    let result = Tokenizer.tokenize("   \t");
    expect.int(List.length(result)).toBe(0);
  });

  test("single word token", ({expect}) => {
    let result = Tokenizer.tokenize("testWord");

    let expectedTokens: list(Tokenizer.t) = [
      {
        text: "testWord",
        startPosition: ZeroBasedIndex(0),
        endPosition: ZeroBasedIndex(8),
      },
    ];

    validateTokens(expect, result, expectedTokens);
  });

  test("single word token, surrounded by whitespace", ({expect}) => {
    let result = Tokenizer.tokenize("  testWord  ");

    let expectedTokens: list(Tokenizer.t) = [
      {
        text: "testWord",
        startPosition: ZeroBasedIndex(2),
        endPosition: ZeroBasedIndex(10),
      },
    ];

    validateTokens(expect, result, expectedTokens);
  });

  test("single letter token, no spaces", ({expect}) => {
    let result = Tokenizer.tokenize("a");

    let expectedTokens: list(Tokenizer.t) = [
      {
        text: "a",
        startPosition: ZeroBasedIndex(0),
        endPosition: ZeroBasedIndex(1),
      },
    ];

    validateTokens(expect, result, expectedTokens);
  });

  test("multiple tokens", ({expect}) => {
    let result = Tokenizer.tokenize(" a btest ");

    let expectedTokens: list(Tokenizer.t) = [
      {
        text: "a",
        startPosition: ZeroBasedIndex(1),
        endPosition: ZeroBasedIndex(2),
      },
      {
        text: "btest",
        startPosition: ZeroBasedIndex(3),
        endPosition: ZeroBasedIndex(8),
      },
    ];

    validateTokens(expect, result, expectedTokens);
  });
});
