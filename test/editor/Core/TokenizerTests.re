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
        startPosition: ZeroBasedPosition(0),
        endPosition: ZeroBasedPosition(8),
      },
    ];

    validateTokens(expect, result, expectedTokens);
  });

  test("single word token, surrounded by whitespace", ({expect}) => {
    let result = Tokenizer.tokenize("  testWord  ");

    let expectedTokens: list(Tokenizer.t) = [
      {
        text: "testWord",
        startPosition: ZeroBasedPosition(2),
        endPosition: ZeroBasedPosition(10),
      },
    ];

    validateTokens(expect, result, expectedTokens);
  });

  test("single letter token, no spaces", ({expect}) => {
    let result = Tokenizer.tokenize("a");

    let expectedTokens: list(Tokenizer.t) = [
      {
        text: "a",
        startPosition: ZeroBasedPosition(0),
        endPosition: ZeroBasedPosition(1),
      },
    ];

    validateTokens(expect, result, expectedTokens);
  });

  test("multiple tokens", ({expect}) => {
    let result = Tokenizer.tokenize(" a btest ");

    let expectedTokens: list(Tokenizer.t) = [
      {
        text: "a",
        startPosition: ZeroBasedPosition(1),
        endPosition: ZeroBasedPosition(2),
      },
      {
        text: "btest",
        startPosition: ZeroBasedPosition(3),
        endPosition: ZeroBasedPosition(8),
      },
    ];

    validateTokens(expect, result, expectedTokens);
  });
});
