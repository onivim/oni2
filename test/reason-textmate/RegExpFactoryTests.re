open TestFramework;

module RegExpFactory = Textmate.RegExpFactory;
module RegExp = Textmate.RegExp;

let createRegex = (~allowBackReferences=true, str) => {
  RegExpFactory.create(~allowBackReferences, str);
};

describe("RegExpFactory", ({describe, _}) => {
  describe("hasAnchors", ({test, _}) => {
    test("returns false if no anchors", ({expect, _}) => {
      let re = createRegex("a|b|c");
      expect.bool(RegExpFactory.hasAnchors(re)).toBe(false);
    });
    test("returns true if has \\A anchor", ({expect, _}) => {
      let re = createRegex("\\A^a|b|c");
      expect.bool(RegExpFactory.hasAnchors(re)).toBe(true);
    });
    test(
      "returns true if has \\A anchor and unresolved back-reference",
      ({expect, _}) => {
      let re = createRegex(~allowBackReferences=false, "\\A^(?!\\1(?=\\S))");
      expect.bool(RegExpFactory.hasAnchors(re)).toBe(true);
    });
  });

  describe("compile", ({test, _}) => {
    test("allowA", ({expect, _}) => {
      let re = createRegex("\\A\\G");
      let re_a0_g0 = RegExpFactory.compile(false, false, re) |> RegExp.raw;
      let re_a1_g0 = RegExpFactory.compile(true, false, re) |> RegExp.raw;
      let re_a1_g1 = RegExpFactory.compile(true, true, re) |> RegExp.raw;
      let re_a0_g1 = RegExpFactory.compile(false, true, re) |> RegExp.raw;
      expect.string(re_a0_g0).toEqual("\\uFFFF\\uFFFF");
      expect.string(re_a1_g0).toEqual("\\A\\uFFFF");
      expect.string(re_a0_g1).toEqual("\\uFFFF\\G");
      expect.string(re_a1_g1).toEqual("\\A\\G");
    })
  });

  describe("hasBackReferences", ({test, _}) => {
    test("returns false if no backreferences", ({expect, _}) => {
      let re = createRegex("a|b|c");
      expect.bool(RegExpFactory.hasBackReferences(re)).toBe(false);
    });
    test("returns true if has backreferences", ({expect, _}) => {
      let re = createRegex(~allowBackReferences=false, "^(?!\\1(?=\\S))");
      expect.bool(RegExpFactory.hasBackReferences(re)).toBe(true);
    });
  });

  describe("supplyReferences", ({test, _}) => {
    test("back references get replaced", ({expect, _}) => {
      let re = createRegex(~allowBackReferences=false, "\\1");

      let newRe = RegExpFactory.supplyReferences([(1, "abc")], re);
      expect.bool(RegExpFactory.hasBackReferences(newRe)).toBe(false);
      expect.string(RegExpFactory.show(newRe)).toEqual("abc");
    })
  });

  describe("escapeRegExpCharacters", ({test, _}) => {
    test("escape characters get replaced", ({expect, _}) => {
      let t = RegExpFactory.escapeRegExpCharacters;

      let validate = (str, expected) => {
        let _ = expect.string(t(str)).toEqual(expected);
        ();
      };

      let cases = [
        ("|", "\\|"),
        ("-", "\\-"),
        ("\\", "\\\\"),
        ("{", "\\{"),
        ("}", "\\}"),
        ("*", "\\*"),
        ("+", "\\+"),
        ("?", "\\?"),
        ("^", "\\^"),
        ("$", "\\$"),
        (".", "\\."),
        (",", "\\,"),
        ("[", "\\["),
        ("]", "\\]"),
        ("(", "\\("),
        (")", "\\)"),
      ];

      List.iter(((e, a)) => validate(e, a), cases);
    })
  });
});
