open TestFramework;

module TreeSitterScopes = Oni_Syntax.TreeSitterScopes;

open TreeSitterScopes;

describe("TreeSitterScopes", ({describe, _}) => {
  describe("TextMateConverter", ({test, _}) => {

    // Create a simple converter... this is a representation of grammars
    // like: https://github.com/atom/language-json/blob/04f1fbd5eb3aabcfc91b30a2c091a9fc657438ee/grammars/tree-sitter-json.cson#L48
    let simpleConverter = TextMateConverter.create([
      ("value", [Scope("source.json")])
    ]);

    test("matches basic scopes", ({expect, _}) => {
      let nonExistentScope  = TextMateConverter.getTextMateScope(
        ~path=["non-existent-tree-sitter-scope"],
        simpleConverter);

      expect.bool(nonExistentScope == None).toBe(true);
    })
  });
});
