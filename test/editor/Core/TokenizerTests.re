open Oni_Core;
open Oni_Core.Types;
open TestFramework;

let validateToken = (expect: Rely__DefaultMatchers.matchers, actualToken: Tokenizer.t, expectedToken: Tokenizer.t) => {
    expect.string(actualToken.text).toEqual(expectedToken.text);   
    expect.int(Position.toZeroBasedIndex(actualToken.startPosition)).toBe(Position.toZeroBasedIndex(expectedToken.startPosition));
    expect.int(Position.toZeroBasedIndex(actualToken.endPosition)).toBe(Position.toZeroBasedIndex(expectedToken.endPosition));
};

let validateTokens = (expect: Rely__DefaultMatchers.matchers, actualTokens: list(Tokenizer.t), expectedTokens: list(Tokenizer.t)) => {
    expect.int(List.length(actualTokens)).toBe(List.length(expectedTokens));

    let f = (actual, expected) => {
        validateToken(expect, actual, expected);    
    };

    List.iter2(f, actualTokens, expectedTokens);
};

describe("tokenize", ({test, _}) => {
    test("empty string", ({expect}) => {
        let result = Tokenizer.tokenize("");
        expect.int(List.length(result)).toBe(0);
    });

    test("single word token", ({expect}) => {
        let result = Tokenizer.tokenize("testWord");

        let expectedTokens: list(Tokenizer.t) = [{
          text: "testWord",
          startPosition: ZeroBasedPosition(0),
          endPosition: ZeroBasedPosition(8)
        }];

        validateTokens(expect, result, expectedTokens);
    });

    test("single word token, surrounded by whitespace", ({expect}) => {
        let result = Tokenizer.tokenize("  testWord  ");

        let expectedTokens: list(Tokenizer.t) = [{
          text: "testWord",
          startPosition: ZeroBasedPosition(2),
          endPosition: ZeroBasedPosition(10)
        }];

        validateTokens(expect, result, expectedTokens);
    });
});
