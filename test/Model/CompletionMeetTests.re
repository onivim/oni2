open TestFramework;

open Oni_Core.Types;

module CompletionMeet = Oni_Model.CompletionMeet;

describe("CompletionMeet", ({describe, _}) => {
  describe("createFromLine", ({test, _}) => {
    let line0column0 = Position.ofInt0(0, 0);
    let line0column1 = Position.ofInt0(0, 1);
    let line0column2 = Position.ofInt0(0, 2);
    let line0column8 = Position.ofInt0(0, 8);

    test("empty line - no meet", ({expect}) => {
      let result =
        CompletionMeet.createFromLine(~cursor=Index.ofInt0(0), "");
      expect.equal(result, None);
    });

    test("single character at beginning", ({expect}) => {
      let result =
        CompletionMeet.createFromLine(~cursor=Index.ofInt0(1), "a");
      expect.equal(result, Some({meet: line0column0, base: "a"}));
    });

    test("spaces prior to character", ({expect}) => {
      let result =
        CompletionMeet.createFromLine(~cursor=Index.ofInt0(1), " a");
      expect.equal(result, Some({meet: line0column1, base: "a"}));
    });

    test("longer base", ({expect}) => {
      let result =
        CompletionMeet.createFromLine(~cursor=Index.ofInt0(4), " abc");
      expect.equal(result, Some({meet: line0column1, base: "abc"}));
    });

    test("default trigger character", ({expect}) => {
      let result =
        CompletionMeet.createFromLine(~cursor=Index.ofInt0(1), " .");
      expect.equal(result, Some({meet: line0column2, base: ""}));
    });

    test("default trigger character with base", ({expect}) => {
      let result =
        CompletionMeet.createFromLine(
          ~cursor=Index.ofInt0(10),
          "console.lo",
        );
      expect.equal(result, Some({meet: line0column8, base: "lo"}));
    });
  })
});
