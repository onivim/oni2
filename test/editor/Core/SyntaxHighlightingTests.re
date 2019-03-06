open Oni_Core;
open Oni_Core.SyntaxHighlighting;
open Oni_Core.TextmateClient.TokenizationResult;
open TestFramework;

/* open Helpers; */

describe("BufferSyntaxHighlight", ({test, _}) => {
  test("updating add tokens", ({expect}) => {
      let shl = BufferSyntaxHighlights.create();

      let sh1 = BufferSyntaxHighlights.update(shl, 1, [{
        line: 0,
        tokens: [ColorizedToken.ofColors(0, 1, 2)],
      }]);

      let lines = BufferSyntaxHighlights.getTokensForLine(sh1, 0);
      expect.int(List.length(lines)).toBe(1);
  });

});
