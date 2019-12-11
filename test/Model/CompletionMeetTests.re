open Oni_Core;
open TestFramework;

module CompletionMeet = Oni_Model.CompletionMeet;

describe("CompletionMeet", ({describe, _}) => {
  describe("createFromLine", ({test, _}) => {
    let line0column0 = Position.ofInt0(0, 0);
    let line0column1 = Position.ofInt0(0, 1);
    let line0column2 = Position.ofInt0(0, 2);
    let line0column8 = Position.ofInt0(0, 8);

    test("empty line - no meet", ({expect}) => {
      let result =
        CompletionMeet.createFromLine(~index=Index.ofInt0(0), "");
      expect.equal(result, None);
    });

    test("single character at beginning", ({expect}) => {
      let result =
        CompletionMeet.createFromLine(~index=Index.ofInt0(1), "a");

      let expected = CompletionMeet.create(~position=line0column0, ~base="a");

      expect.equal(result, Some(expected));
    });

    test("spaces prior to character", ({expect}) => {
      let result =
        CompletionMeet.createFromLine(~index=Index.ofInt0(1), " a");
      
      let expected = CompletionMeet.create(~position=line0column1, ~base="a");
      expect.equal(result, Some(expected));
    });

    test("longer base", ({expect}) => {
      let result =
        CompletionMeet.createFromLine(~index=Index.ofInt0(4), " abc");
      let expected = CompletionMeet.create(~position=line0column1, ~base="abc");
      expect.equal(result, Some(expected));
    });

    test("default trigger character", ({expect}) => {
      let result =
        CompletionMeet.createFromLine(~index=Index.ofInt0(1), " .");
      let expected = CompletionMeet.create(~position=line0column2, ~base="");
      expect.equal(result, Some(expected));
    });

    test("default trigger character with base", ({expect}) => {
      let result =
        CompletionMeet.createFromLine(
          ~index=Index.ofInt0(10),
          "console.lo",
        );
      let expected = CompletionMeet.create(~position=line0column8, ~base="lo");
      expect.equal(result, Some(expected));
    });
  })
});
