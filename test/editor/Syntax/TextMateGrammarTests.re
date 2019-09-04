open TestFramework;

open Oniguruma;

module TextMateGrammar = Oni_Syntax.TextMateGrammar;

describe("TextMateGrammar", ({describe, _}) => {
  /* Test case inspired by:
    https://code.visualstudio.com/api/language-extensions/syntax-highlight-guide
   */


  let grammar = TextMateGrammar.create(
    ~scopeName="source.abc",
    ~patterns=[Include("#expression")],
    ~repository=[
      ("expression", [Include("#letter"), Include("#word"), Include("#paren-expression")]),
      ("letter", [Match({
        matchRegex: OnigRegExp.create("a|b|c"),
        matchName: "keyword.letter",
        captures: [],
      })]),
      ("word", [Match({
        matchRegex: OnigRegExp.create("def"),
        matchName: "keyword.word",
        captures: [],
      })]),
      ("paren-expression", [MatchRange(
      {
        beginRegex: OnigRegExp.create("\\("),
        endRegex: OnigRegExp.create("\\)"),
        beginCaptures: [(0, "punctuation.paren.open")],
        endCaptures: [(0, "punctuation.paren.close")],
        matchRangeName: "expression.group",
        patterns: [Include("#expression")]
      })])
      ],
      () 
  );

  describe("tokenize", ({test, _}) => {

    test("simple letter token", ({expect, _}) => {
      let (tokens, _) = TextMateGrammar.tokenize(~grammar, "a")
      expect.int(List.length(tokens)).toBe(1);

      let firstToken = List.hd(tokens);
      expect.bool(firstToken.scopes == ["keyword.letter", "source.abc"]).toBe(true);
      expect.int(firstToken.position).toBe(0);
      expect.int(firstToken.length).toBe(1);
    });
    test("simple word tokens", ({expect, _}) => {
      let (tokens, _) = TextMateGrammar.tokenize(~grammar, " def")
      expect.int(List.length(tokens)).toBe(1);

      let firstToken = List.hd(tokens);
      expect.bool(firstToken.scopes == ["keyword.word", "source.abc"]).toBe(true);
      expect.int(firstToken.position).toBe(1);
      expect.int(firstToken.length).toBe(3);
    });
    test("different tokens", ({expect, _}) => {
      let (tokens, _) = TextMateGrammar.tokenize(~grammar, " adef b")
      expect.int(List.length(tokens)).toBe(3);

      let firstToken = List.hd(tokens);
      expect.bool(firstToken.scopes == ["keyword.letter", "source.abc"]).toBe(true);
      expect.int(firstToken.position).toBe(1);
      expect.int(firstToken.length).toBe(1);
      
      let secondToken = List.nth(tokens, 1);
      expect.bool(secondToken.scopes == ["keyword.word", "source.abc"]).toBe(true);
      expect.int(secondToken.position).toBe(2);
      expect.int(secondToken.length).toBe(3);
      
      let thirdToken = List.nth(tokens, 2);
      expect.bool(thirdToken.scopes == ["keyword.letter", "source.abc"]).toBe(true);
      expect.int(thirdToken.position).toBe(6);
      expect.int(thirdToken.length).toBe(1);
    });
  });
});
