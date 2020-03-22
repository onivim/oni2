open Oni_Core;
open TestFramework;

let tab2 = IndentationSettings.create(~mode=Tabs, ~size=2, ~tabSize=2, ());
let space2 =
  IndentationSettings.create(~mode=Spaces, ~size=2, ~tabSize=2, ());
let tab4 = IndentationSettings.create(~mode=Tabs, ~size=4, ~tabSize=4, ());
let space4 =
  IndentationSettings.create(~mode=Spaces, ~size=4, ~tabSize=4, ());

describe("Indentation", ({describe, _}) =>
  describe("getLevel", ({test, _}) => {
    test("string with no tabs / spaces has level 0", ({expect, _}) => {
      let level = Indentation.getLevel(tab2, "test string");
      expect.int(level).toBe(0);
    });

    test("string with 1 tab has indentation level of 1", ({expect, _}) => {
      let a1 = Indentation.getLevel(tab2, "\ttest string");
      let a2 = Indentation.getLevel(space4, "\ttest string");

      expect.int(a1).toBe(1);
      expect.int(a2).toBe(1);
    });
    test(
      "string with 2 tab has indentation level of 2, regardless of setting",
      ({expect, _}) => {
      let a1 = Indentation.getLevel(tab2, "\t\ttest string");
      let a2 = Indentation.getLevel(tab4, "\t\ttest string");
      let a3 = Indentation.getLevel(space2, "\t\ttest string");

      expect.int(a1).toBe(2);
      expect.int(a2).toBe(2);
      expect.int(a3).toBe(2);
    });
    test("string with 4 spaces has proper indentation level", ({expect, _}) => {
      let a1 = Indentation.getLevel(space4, "    test string");
      let a2 = Indentation.getLevel(space2, "    test string");
      let a3 = Indentation.getLevel(tab2, "    test string");
      let a4 = Indentation.getLevel(tab4, "    test string");

      expect.int(a1).toBe(1);
      expect.int(a2).toBe(2);
      expect.int(a3).toBe(2);
      expect.int(a4).toBe(1);
    });
    test("extra space bumps up to next indentation level", ({expect, _}) => {
      let a1 = Indentation.getLevel(space4, "     test string");

      expect.int(a1).toBe(2);
    });
    test("string with single space has one indentation levej", ({expect, _}) => {
      let a1 = Indentation.getLevel(space4, " test string");
      let a2 = Indentation.getLevel(space2, " test string");
      let a3 = Indentation.getLevel(tab2, " test string");
      let a4 = Indentation.getLevel(tab4, " test string");

      expect.int(a1).toBe(1);
      expect.int(a2).toBe(1);
      expect.int(a3).toBe(1);
      expect.int(a4).toBe(1);
    });
    test("doesn't count tabs after a character", ({expect, _}) => {
      let a1 = Indentation.getLevel(space4, "\ta\t");
      let a2 = Indentation.getLevel(tab2, "\t\tabc\t\t");

      expect.int(a1).toBe(1);
      expect.int(a2).toBe(2);
    });
    test("if all whitespace, indentation level should be 0", ({expect, _}) => {
      let a1 = Indentation.getLevel(space4, "\t    \t\t");
      let a2 = Indentation.getLevel(tab2, "\t\t       ");

      expect.int(a1).toBe(0);
      expect.int(a2).toBe(0);
    });
  })
);
