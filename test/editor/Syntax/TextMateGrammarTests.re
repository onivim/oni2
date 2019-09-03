open TestFramework;

open Oniguruma;

module TextMateGrammar = Oni_Syntax.TextMateGrammar;

describe("TextMateGrammar", ({describe, _}) => {
  /* Test case inspired by:
    https://code.visualstudio.com/api/language-extensions/syntax-highlight-guide
   */


  let _simpleGrammar = TextMateGrammar.create(
    ~scopeName="source.abc",
    ~patterns=[Include("#expression")],
    ~repository=[
      ("expression", [Include("#letter"), Include("#paren-expression")]),
      ("letter", [Match({
        matchRegex: OnigRegExp.create("a|b|c"),
        matchName: "keyword.letter",
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

    test("simple tokens", ({expect, _}) => {
      expect.int(0).toBe(1);
    });
  });

});
