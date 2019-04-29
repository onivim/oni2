open Oni_Core;
open Oni_Model;
open Oni_Extensions;

open Helpers;

describe("BufferWrap", ({describe, test, _}) => {
  describe("indentation settings", ({test, _}) => {
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
          Colors.white,
          Colors.black,
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
    });
  });
});
