open TestFramework;

module TreeSitterScopes = Oni_Syntax.TreeSitterScopes;

open TreeSitterScopes;

describe("TreeSitterScopes", ({describe, _}) => {
  describe("Selector", ({test, _}) => {
    test("returns correct results", ({expect, _}) => {
      expect.bool(Selector.checkChildSelector("pair") == None).toBe(true);
      expect.bool(
        Selector.checkChildSelector("pair:nth-child(0)") == Some("0"),
      ).
        toBe(
        true,
      );
      expect.bool(
        Selector.checkChildSelector("pair:nth-child(99)") == Some("99"),
      ).
        toBe(
        true,
      );
      expect.bool(
        Selector.checkChildSelector("pair:first-child") == Some("0"),
      ).
        toBe(
        true,
      );
    });
    test("parse", ({expect, _}) => {
      expect.bool(Selector.parse("pair > string:nth-child(0)") == (["string", "pair"], Some("0"))).toBe(true);
    });
  });
  describe("TextMateConverter", ({test, _}) => {
    // Create a simple converter... this is a representation of grammars
    // like: https://github.com/atom/language-json/blob/04f1fbd5eb3aabcfc91b30a2c091a9fc657438ee/grammars/tree-sitter-json.cson#L48
    let simpleConverter =
      TextMateConverter.create([
        ("value", [Scope("source.json")]),
        ("object", [Scope("meta.structure.dictionary.json")]),
        (
          "string_content",
          [
            RegExMatch(
              Str.regexp("^http:\\/\\/"),
              "markup.underline.link.http.hyperlink",
            ),
            RegExMatch(
              Str.regexp("^https:\\/\\/"),
              "markup.underline.link.https.hyperlink",
            ),
          ],
        ),
        ("pair > string", [Scope("constant.language")]),
        (
          "pair > string:nth-child(1)",
          [Scope("string.quoted.dictionary.key.json")],
        ),
      ]);

    test("returns None for non-existent scopes", ({expect, _}) => {
      let nonExistentScope =
        TextMateConverter.getTextMateScope(
          ~path=["non-existent-tree-sitter-scope"],
          simpleConverter,
        );

      expect.bool(nonExistentScope == None).toBe(true);
    });
    test("matches simple scopes", ({expect, _}) => {
      let scope =
        TextMateConverter.getTextMateScope(~path=["value"], simpleConverter);

      expect.bool(scope == Some("source.json")).toBe(true);
    });
    test("matches another simple scope", ({expect, _}) => {
      let scope =
        TextMateConverter.getTextMateScope(
          ~path=["object"],
          simpleConverter,
        );

      expect.bool(scope == Some("meta.structure.dictionary.json")).toBe(
        true,
      );
    });
    test("regex match failure", ({expect, _}) => {
      let scope =
        TextMateConverter.getTextMateScope(
          ~path=["string_content"],
          ~token="derp",
          simpleConverter,
        );

      expect.bool(scope == None).toBe(true);
    });
    test("regex match success", ({expect, _}) => {
      let scope =
        TextMateConverter.getTextMateScope(
          ~path=["string_content"],
          ~token="https://v2.onivim.io",
          simpleConverter,
        );

      expect.bool(scope == Some("markup.underline.link.https.hyperlink")).
        toBe(
        true,
      );
    });
    test("child selector", ({expect, _}) => {
      let scope =
        TextMateConverter.getTextMateScope(
          ~path=["string", "pair"],
          simpleConverter,
        );

      expect.bool(scope == Some("constant.language")).toBe(true);
    });
    test("child selector only matches direct children", ({expect, _}) => {
      let scope =
        TextMateConverter.getTextMateScope(
          ~path=["string", "random-scope", "pair"],
          simpleConverter,
        );

      expect.bool(scope == None).toBe(true);
    });
    test("child selector matches descendants", ({expect, _}) => {
      let scope =
        TextMateConverter.getTextMateScope(
          ~path=["string", "pair", "random-scope"],
          simpleConverter,
        );

      expect.bool(scope == Some("constant.language")).toBe(true);
    });
    test("nth-child(1) selector matches", ({expect, _}) => {
      let scope =
        TextMateConverter.getTextMateScope(
          ~index=1,
          ~path=["string", "pair", "random-scope"],
          simpleConverter,
        );

      expect.bool(scope == Some("string.quoted.dictionary.key.json")).toBe(true);
    });
  });
});
