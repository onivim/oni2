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

    test("indeterminate uses passed-in settings", ({expect}) => {
      let settings = guessIndentationArray(indeterminateLines, 4, true);
      expect.bool(settings.mode == IndentationSettings.Spaces).toBe(true);

      let settings = guessIndentationArray(indeterminateLines, 4, false);
      expect.bool(settings.mode == IndentationSettings.Tabs).toBe(true);
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
      expect.int(r.size).toBe(1);
    });

    test("mostly double spaced", ({expect}) => {
      let r = guessIndentationArray(mostlyDoubleSpaced, 4, true);
      expect.bool(r.mode == Spaces).toBe(true);
      expect.int(r.size).toBe(2);
    });
  })
);
