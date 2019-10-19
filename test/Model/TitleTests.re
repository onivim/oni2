open TestFramework;

// open Oni_Core.Types;

module Title = Oni_Model.Title;

describe("Title", ({describe, _}) => {
  describe("parse", ({test, _}) => {
    test("plain string", ({expect}) => {
      let title = Title.ofString("abc");
      expect.equal(title, [Title.Text("abc")]);
    });
    test("string with separator", ({expect}) => {
      let title = Title.ofString("abc${separator}def");
      expect.equal(
        title,
        [Title.Text("abc"), Title.Separator, Title.Text("def")],
      );
    });
  })
});
