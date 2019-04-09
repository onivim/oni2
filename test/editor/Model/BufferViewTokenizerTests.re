open Revery;
open Oni_Core;
open Oni_Model;
open Oni_Extensions;
open TestFramework;

open Helpers;

let theme = Theme.create();

let tokenColors = [];
let colorMap = ColorMap.create();

describe("tokenize", ({test, _}) => {
  test("empty string", ({expect}) => {
    let result =
      BufferViewTokenizer.tokenize("", theme, tokenColors, colorMap);
    expect.int(List.length(result)).toBe(0);
  });

  describe("tokenize", ({test, describe, _}) => {

/*   describe("indentation settings", ({test, _}) => { */
/*     test("accounts for tab size", ({expect}) => { */
/*          let indentation = IndentationSettings.create(~mode=Tabs, ~size=2, ~tabSize=4, ()); */
/*          let result = Tokenizer.tokenize(~indentation, "\tabc", theme, tokenColors, colorMap); */

         
/*          let expectedTokens: list(Tokenizer.t) = [ */
/*          { */
/*             text: "abc", */
/*             startPosition:ZeroBasedIndex(4), */
/*             endPosition: ZeroBasedIndex(7), */
/*             color: Colors.red, */
/*          } */ 
/*          ]; */

/*          validateTokens(expect, result, expectedTokens); */
/*     }); */
/*   }); */

  test("string with only whitespace", ({expect}) => {
    let result =
      BufferViewTokenizer.tokenize("   \t", theme, tokenColors, colorMap);
    expect.int(List.length(result)).toBe(0);
  });

  test("single word token", ({expect}) => {
    let result =
      BufferViewTokenizer.tokenize("testWord", theme, tokenColors, colorMap);

    let expectedTokens: list(BufferViewTokenizer.t) = [
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
    let result =
      BufferViewTokenizer.tokenize(
        "  testWord  ",
        theme,
        tokenColors,
        colorMap,
      );

    let expectedTokens: list(BufferViewTokenizer.t) = [
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
    let result =
      BufferViewTokenizer.tokenize("a", theme, tokenColors, colorMap);

    let expectedTokens: list(BufferViewTokenizer.t) = [
      {
        text: "a",
        startPosition: ZeroBasedIndex(0),
        endPosition: ZeroBasedIndex(1),
        color: Colors.red,
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
      BufferViewTokenizer.tokenize("ab", theme, tokenColors, colorMap);

    let expectedTokens: list(BufferViewTokenizer.t) = [
      {
        text: "a",
        startPosition: ZeroBasedIndex(0),
        endPosition: ZeroBasedIndex(1),
        color: Colors.red,
      },
      {
        text: "b",
        startPosition: ZeroBasedIndex(1),
        endPosition: ZeroBasedIndex(2),
        color: Colors.red,
      },
    ];

    validateTokens(expect, result, expectedTokens);
  });

  test("multiple tokens", ({expect}) => {
    let result =
      BufferViewTokenizer.tokenize(" a btest ", theme, tokenColors, colorMap);

    let expectedTokens: list(BufferViewTokenizer.t) = [
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
