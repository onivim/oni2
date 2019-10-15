open TestFramework;

open Oni_Core.Types;

module CompletionMeet = Oni_Model.CompletionMeet;

describe("CompletionMeet", ({describe, _}) => {
  describe("getMeetFromLine", ({test, _}) => {
    test("empty line - no meet", ({expect}) => {
      let result = getMeetFromLine(~cursor=Index.ofInt0(0), "");
      expect.equal(result, None);
    });
  });
});
