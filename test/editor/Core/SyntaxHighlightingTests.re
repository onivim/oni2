open Oni_Core;
open Oni_Core.SyntaxHighlighting;
open Oni_Core.TextmateClient.TokenizationResult;
open TestFramework;

/* open Helpers; */

let token = ColorizedToken.create(0, 0);

let oneToken = [token];
let twoTokens = [token, token];
let threeTokens = [token, token, token];
let fourTokens = [token, token, token, token];

let threeTokenBuffer = BufferSyntaxHighlights.create();
let threeTokenBuffer = BufferSyntaxHighlights.update(threeTokenBuffer, 1, [{
        line: 0,
        tokens: oneToken,
      }, {
        line: 1,
        tokens: twoTokens,
      }, {
        line: 2, 
        tokens: threeTokens,
      }]);

describe("BufferSyntaxHighlight", ({test, _}) => {
  test("updating add tokens", ({expect}) => {
      let shl = BufferSyntaxHighlights.create();

      let sh1 = BufferSyntaxHighlights.update(shl, 1, [{
        line: 0,
        tokens: oneToken,
      }]);

      let tokens = BufferSyntaxHighlights.getTokensForLine(sh1, 0);
      expect.int(List.length(tokens)).toBe(1);
  });

  test("no shift if no lines added or deleted", ({expect}) => {

      let sh1 = BufferSyntaxHighlights.shift(threeTokenBuffer, 1, 1, 0);

      let tokens0 = BufferSyntaxHighlights.getTokensForLine(sh1, 0);
      let tokens1 = BufferSyntaxHighlights.getTokensForLine(sh1, 1);
      let tokens2 = BufferSyntaxHighlights.getTokensForLine(sh1, 2);

      /* There should be no shift */
      expect.int(List.length(tokens0)).toBe(1);
      expect.int(List.length(tokens1)).toBe(2);
      expect.int(List.length(tokens2)).toBe(3);
  });

  test("add lines between 1 and 2", ({expect}) => {
    
      let sh1 = BufferSyntaxHighlights.shift(threeTokenBuffer, 1, 1, 2);

      let tokens0 = BufferSyntaxHighlights.getTokensForLine(sh1, 0);
      let tokens1 = BufferSyntaxHighlights.getTokensForLine(sh1, 1);
      let tokens2 = BufferSyntaxHighlights.getTokensForLine(sh1, 2);
      let tokens3 = BufferSyntaxHighlights.getTokensForLine(sh1, 3);
      let tokens4 = BufferSyntaxHighlights.getTokensForLine(sh1, 4);

      expect.int(List.length(tokens0)).toBe(1);
      expect.int(List.length(tokens1)).toBe(0);
      expect.int(List.length(tokens2)).toBe(0);
      expect.int(List.length(tokens3)).toBe(2);
      expect.int(List.length(tokens4)).toBe(3);
  });
  test("remove line", ({expect}) => {
    
      let sh1 = BufferSyntaxHighlights.shift(threeTokenBuffer, 1, 1, -1);

      let tokens0 = BufferSyntaxHighlights.getTokensForLine(sh1, 0);
      let tokens1 = BufferSyntaxHighlights.getTokensForLine(sh1, 1);

      expect.int(List.length(tokens0)).toBe(1);
      expect.int(List.length(tokens1)).toBe(3);
  });

});
