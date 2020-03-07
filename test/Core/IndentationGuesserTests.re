open Oni_Core;
open TestFramework;

open IndentationGuesser;

describe("IndentationGuesser", ({describe, _}) =>
  describe("guessIndentationArray", ({test, _}) => {
    let indeterminateLines = [|"abc", "def", "ghi"|];

    let moreTabsThanSpaces = [|"\tabc", "  def", "\tghi"|];

    let someEmptyLines = [|"  ", "\tdef", "  "|];

    let moreSpacesThanTabs = [|"  abc", "  def", "  ghi"|];

    let mostlySingleSpaced = [|
      " abc",
      "  def",
      "   ghi",
      "  ghi",
      "    ghi",
      "   ghi",
    |];

    let mostlyDoubleSpaced = [|
      "  abc",
      "    def",
      "  ghi",
      "  ghi",
      "   ghi",
      "      ghi",
    |];

    let largerExample = [|
      "module Index = {",
      "  [@deriving show({with_path: false})]",
      "  type t =",
      "    | ZeroBasedIndex(int)",
      "    | OneBasedIndex(int);",
      "",
      "  let toZeroBasedInt = (pos: t) =>",
      "    switch (pos) {",
      "    | ZeroBasedIndex(n) => n",
      "    | OneBasedIndex(n) => n - 1",
      "    };",
      "",
      "  let toInt0 = toZeroBasedInt;",
      "",
      "  let toOneBasedInt = (pos: t) =>",
      "    switch (pos) {",
      "    | ZeroBasedIndex(n) => n + 1",
      "    | OneBasedIndex(n) => n",
      "    };",
      "",
      "  let toInt1 = toOneBasedInt;",
      "};",
      "",
      "module EditorSize = {",
      "  [@deriving show({with_path: false})]",
      "  type t = {",
      "    pixelWidth: int,",
      "    pixelHeight: int,",
      "  };",
      "",
      "  let create = (~pixelWidth: int, ~pixelHeight: int, ()) => {",
      "    pixelWidth,",
      "    pixelHeight,",
      "  };",
      "};",
    |];

    let singleSpaceBlockComment = [|
      "/*****",
      " *",
      " * Foo",
      " *",
      " *",
      " */",
    |];

    test("indeterminate uses passed-in settings", ({expect}) => {
      let settings = guessIndentationArray(indeterminateLines, 4, true);
      expect.bool(settings.mode == IndentationSettings.Spaces).toBe(true);
      expect.int(settings.size).toBe(4);

      let settings = guessIndentationArray(indeterminateLines, 3, false);
      expect.bool(settings.mode == IndentationSettings.Tabs).toBe(true);
      expect.int(settings.size).toBe(3);
    });

    test("more tabs than spaces", ({expect}) => {
      let r = guessIndentationArray(moreTabsThanSpaces, 4, true);
      expect.bool(r.mode == Tabs).toBe(true);
    });

    test("more spaces than tabs", ({expect}) => {
      let r = guessIndentationArray(moreSpacesThanTabs, 4, false);
      expect.bool(r.mode == Spaces).toBe(true);
    });

    test("ignores empty lines", ({expect}) => {
      let r = guessIndentationArray(someEmptyLines, 4, true);
      expect.bool(r.mode == Tabs).toBe(true);
    });

    test("mostly single spaced", ({expect}) => {
      let r = guessIndentationArray(mostlySingleSpaced, 4, true);
      expect.bool(r.mode == Spaces).toBe(true);
      expect.int(r.size).toBe(2);
    });

    test("mostly double spaced", ({expect}) => {
      let r = guessIndentationArray(mostlyDoubleSpaced, 4, true);
      expect.bool(r.mode == Spaces).toBe(true);
      expect.int(r.size).toBe(2);
    });

    test("larger example", ({expect}) => {
      let r = guessIndentationArray(largerExample, 4, false);
      expect.bool(r.mode == Spaces).toBe(true);
      expect.int(r.size).toBe(2);
    });

    test("single-space block comment", ({expect}) => {
      let r = guessIndentationArray(singleSpaceBlockComment, 4, false);
      expect.bool(r.mode == Spaces).toBe(true);
      expect.int(r.size).toBe(2);
    });
  })
);
