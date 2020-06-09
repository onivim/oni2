open EditorCoreTypes;
open Oni_Core;
open TestFramework;

let zeroLocation = Location.create(~line=Index.zero, ~column=Index.zero);
let zeroRange = Range.{start: zeroLocation, stop: zeroLocation};

describe("SingleEdit", ({describe, _}) => {
  describe("applyEdit", ({test, _}) => {
    test("no-op makes no changes", ({expect, _}) => {
      let lines = [|"Hello!"|];
      let noopEdit = SingleEdit.{text: None, range: zeroRange};

      let outLines = SingleEdit.applyEdit(~lines, noopEdit);
      expect.equal(outLines, lines);
    });
    test("insert character at beginning of line", ({expect, _}) => {
      let lines = [|"Hello!"|];
      let noopEdit = SingleEdit.{text: Some("a"), range: zeroRange};

      let outLines = SingleEdit.applyEdit(~lines, noopEdit);
      expect.equal(outLines, [|"aHello!"|]);
    });
    test("remove character at end of line", ({expect, _}) => {
      let lines = [|"Hello!"|];
      let noopEdit =
        SingleEdit.{
          text: Some(""),
          range:
            Range.{
              start:
                Location.create(~line=Index.zero, ~column=Index.(zero + 5)),
              stop:
                Location.create(~line=Index.zero, ~column=Index.(zero + 6)),
            },
        };

      let outLines = SingleEdit.applyEdit(~lines, noopEdit);
      expect.equal(outLines, [|"Hello"|]);
    });
  })
});
