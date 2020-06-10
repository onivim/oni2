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

let range  = (startLine, startColumn, endLine, endColumn) => {
  Range.{
    start: Location.create(
      ~line=Index.(zero + startLine),
      ~column=Index.(zero + startColumn)
    ),
    stop:
      Location.create(
        ~line= Index.(zero+endLine),
        ~column= Index.(zero+endColumn),
      )
  }
};

let edit = (~text, range) => {
  Edit.{
    text,
    range,
  };
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
          range: range(0, 5, 0, 6),
        };

      let result: Edit.editResult =
        Edit.applyEdit(~provider, edit) |> Result.get_ok;
      expect.equal(result.newLines, [|"Hello"|]);
    });
  })
  describe("sort", ({test, _}) => {
    test("already sorted should be as-is", ({expect, _}) => {
      let first = range(0, 1, 0, 1) |> edit(~text=None);
      let second = range(0, 0, 0, 0) |> edit(~text=None);

      let edits = [first, second];
      let sortedEdits = Edit.sort(edits);

      expect.equal(sortedEdits, edits);
    });
    test("should get sorted", ({expect, _}) => {
      let first = range(6, 0, 6, 0) |> edit(~text=None);
      let second = range(6, 3, 6, 4) |> edit(~text=None);

      let edits = [first, second];
      let sortedEdits = Edit.sort(edits);
      let expectedEdits = [second, first];

      expect.equal(sortedEdits, expectedEdits);
    });
  })
});
