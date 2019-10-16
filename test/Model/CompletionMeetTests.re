open TestFramework;

open Oni_Core.Types;

module CompletionMeet = Oni_Model.CompletionMeet;

describe("CompletionMeet", ({describe, _}) => {
  describe("getMeetFromLine", ({test, _}) => {
    test("empty line - no meet", ({expect}) => {
      let result = CompletionMeet.getMeetFromLine(~cursor=Index.ofInt0(0), "");
      expect.equal(result, None);
    });
    
    test("single character at beginning", ({expect}) => {
      let result = CompletionMeet.getMeetFromLine(~cursor=Index.ofInt0(1), "a");
      expect.equal(result, Some({index: 0, base: "a"}));
    });
    
    test("spaces prior to character", ({expect}) => {
      let result = CompletionMeet.getMeetFromLine(~cursor=Index.ofInt0(1), " a");
      expect.equal(result, Some({index: 1, base: "a"}));
    });
    
    test("longer base", ({expect}) => {
      let result = CompletionMeet.getMeetFromLine(~cursor=Index.ofInt0(4), " abc");
      expect.equal(result, Some({index: 1, base: "abc"}));
    });
    
    test("default trigger character", ({expect}) => {
      let result = CompletionMeet.getMeetFromLine(~cursor=Index.ofInt0(1), " .");
      expect.equal(result, Some({index: 2, base: ""}));
    });
    
    test("default trigger character with base", ({expect}) => {
      let result = CompletionMeet.getMeetFromLine(~cursor=Index.ofInt0(10), "console.lo");
      expect.equal(result, Some({index: 8, base: "lo"}));
    });
  });
});
