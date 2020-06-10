open EditorCoreTypes;
open TestFramework;
open Vim;

let zeroLocation = Location.create(~line=Index.zero, ~column=Index.zero);
let zeroRange = Range.{start: zeroLocation, stop: zeroLocation};

let arrayProvider = (strArray, idx) =>
  if (idx >= Array.length(strArray) || idx < 0) {
    None;
  } else {
    Some(strArray[idx]);
  };

describe("Edit", ({describe, _}) => {
  describe("applyEdit", ({test, _}) => {
    test("no-op makes no changes", ({expect, _}) => {
      let provider = [|"Hello!"|] |> arrayProvider;
      let noopEdit = Edit.{text: None, range: zeroRange};

      let result: Edit.editResult =
        Edit.applyEdit(~provider, noopEdit) |> Result.get_ok;
      expect.equal(result.newLines, [|"Hello!"|]);
    });
    test("insert character at beginning of line", ({expect, _}) => {
      let provider = [|"Hello!"|] |> arrayProvider;
      let edit = Edit.{text: Some("a"), range: zeroRange};

      let result: Edit.editResult =
        Edit.applyEdit(~provider, edit) |> Result.get_ok;
      expect.equal(result.newLines, [|"aHello!"|]);
    });
    test("remove character at end of line", ({expect, _}) => {
      let provider = [|"Hello!"|] |> arrayProvider;
      let edit =
        Edit.{
          text: Some(""),
          range:
            Range.{
              start:
                Location.create(~line=Index.zero, ~column=Index.(zero + 5)),
              stop:
                Location.create(~line=Index.zero, ~column=Index.(zero + 6)),
            },
        };

      let result: Edit.editResult =
        Edit.applyEdit(~provider, edit) |> Result.get_ok;
      expect.equal(result.newLines, [|"Hello"|]);
    });
  })
});
