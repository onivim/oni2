open TestFramework;

module TreeSitterScopes = Oni_Syntax.TreeSitterScopes;

open TreeSitterScopes;

describe("TreeSitterScopes", ({describe, _}) => {
  describe("TextMateConverter", ({test, _}) => {

    // Create a simple converter... this is a representation of grammars
    // like: https://github.com/atom/language-json/blob/04f1fbd5eb3aabcfc91b30a2c091a9fc657438ee/grammars/tree-sitter-json.cson#L48
    let simpleConverter = TextMateConverter.create([
      ("value", [Scope("source.json")]),
      ("object", [Scope("meta.structure.dictionary.json")]),
      ("string_content", [
        RegExMatch("^http:\/\/", scopes: "markup.underline.link.http.hyperlink"),
        RegExMatch("^https:\/\/", scopes: "markup.underline.link.https.hyperlink"),
      ])
    ]);

    test("returns None for non-existent scopes", ({expect, _}) => {
      let nonExistentScope  = TextMateConverter.getTextMateScope(
        ~path=["non-existent-tree-sitter-scope"],
        simpleConverter);

      expect.bool(nonExistentScope == None).toBe(true);
    });
    test("matches simple scopes", ({expect, _}) => {
      let scope  = TextMateConverter.getTextMateScope(
        ~path=["value"],
        simpleConverter);

      expect.bool(scope == Some("source.json")).toBe(true);
    });
    test("matches another simple scope", ({expect, _}) => {
      let scope  = TextMateConverter.getTextMateScope(
        ~path=["object"],
        simpleConverter);

      expect.bool(scope == Some("meta.structure.dictionary.json")).toBe(true);
    })
  });
});
