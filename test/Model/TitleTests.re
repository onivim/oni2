open TestFramework;

open Oni_Core;

module Title = Oni_Model.Title;

describe("Title", ({describe, _}) => {
  describe("parse", ({test, _}) => {
    test("plain string", ({expect}) => {
      let title = Title.ofString("abc");
      expect.equal(title, [Title.Text("abc", false)]);
    });
    test("string with separator", ({expect}) => {
      let title = Title.ofString("abc${separator}def");
      expect.equal(
        title,
        [
          Title.Text("abc", false),
          Title.Separator,
          Title.Text("def", false),
        ],
      );
    });
    test("string with variables", ({expect}) => {
      let title = Title.ofString("${variable1}${separator}${variable2}");
      expect.equal(
        title,
        [
          Title.Variable("variable1"),
          Title.Separator,
          Title.Variable("variable2"),
        ],
      );
    });
  });

  describe("toString", ({test, _}) => {
    let simpleMap =
      [("variable1", "rv1"), ("variable2", "rv2")]
      |> List.to_seq
      |> StringMap.of_seq;

    test("basic case", ({expect}) => {
      let title =
        Title.ofString("prefix${variable1}${separator}${variable2}postfix");
      let result = Title.toString(title, simpleMap);
      expect.string(result).toEqual("prefixrv1 - rv2postfix");
    });
    test("nested variable missing case", ({expect}) => {
      let title =
        Title.ofString(
          "prefix${variable1}${separator}${missingVariable}${separator}${variable2}postfix",
        );
      let result = Title.toString(title, simpleMap);
      expect.string(result).toEqual("prefixrv1 - rv2postfix");
    });
  });
});
