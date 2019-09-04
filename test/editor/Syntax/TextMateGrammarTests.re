open TestFramework;

open Oniguruma;

module TextMateGrammar = Oni_Syntax.TextMateGrammar;

describe("TextMateGrammar", ({describe, _}) => {
  /* Test case inspired by:
      https://code.visualstudio.com/api/language-extensions/syntax-highlight-guide
     */

  let grammar =
    TextMateGrammar.create(
      ~scopeName="source.abc",
      ~patterns=[Include("#expression")],
      ~repository=[
        (
          "expression",
          [
            Include("#letter"),
            Include("#word"),
            Include("#capture-groups"),
            Include("#paren-expression"),
          ],
        ),
        (
          "letter",
          [
            Match({
              matchRegex: OnigRegExp.create("a|b|c"),
              matchName: "keyword.letter",
              captures: [],
            }),
          ],
        ),
        (
          "word",
          [
            Match({
              matchRegex: OnigRegExp.create("def"),
              matchName: "keyword.word",
              captures: [],
            }),
          ],
        ),
        (
          "capture-groups",
          [
            Match({
              matchRegex: OnigRegExp.create("(@selector\\()(.*?)(\\))"),
              matchName: "",
              captures: [
                (1, "storage.type.objc"),
                (3, "storage.type.objc"),
              ],
            }),
          ],
        ),
        (
          "paren-expression",
          [
            MatchRange({
              beginRegex: OnigRegExp.create("\\("),
              endRegex: OnigRegExp.create("\\)"),
              beginCaptures: [(0, "punctuation.paren.open")],
              endCaptures: [(0, "punctuation.paren.close")],
              matchRuleName: "#paren-expression",
              matchScopeName: "expression.group",
              patterns: [Include("#expression")],
            }),
          ],
        ),
      ],
      (),
    );

  describe("tokenize", ({test, describe, _}) => {
    describe("begin / end rules", ({test, _}) => {
      test("multi-line begin/end", ({expect, _}) => {
        let (line1Token, line1Scope) =
          TextMateGrammar.tokenize(~grammar, "(");
        let (line2Token, line2Scope) =
          TextMateGrammar.tokenize(~grammar, ~scopes=Some(line1Scope), "a");
        let (line3Token, _) =
          TextMateGrammar.tokenize(~grammar, ~scopes=Some(line2Scope), ")");
        expect.int(List.length(line1Token)).toBe(1);
        expect.int(List.length(line2Token)).toBe(1);
        expect.int(List.length(line3Token)).toBe(1);

        let firstToken = List.hd(line1Token);
        expect.bool(
          firstToken.scopes
          == ["punctuation.paren.open", "expression.group", "source.abc"],
        ).
          toBe(
          true,
        );
        expect.int(firstToken.position).toBe(0);
        expect.int(firstToken.length).toBe(1);

        let secondToken = List.hd(line2Token);
        expect.bool(
          secondToken.scopes
          == ["keyword.letter", "expression.group", "source.abc"],
        ).
          toBe(
          true,
        );
        expect.int(secondToken.position).toBe(0);
        expect.int(secondToken.length).toBe(1);

        let thirdToken = List.hd(line3Token);
        expect.bool(
          thirdToken.scopes
          == ["punctuation.paren.close", "expression.group", "source.abc"],
        ).
          toBe(
          true,
        );
        expect.int(thirdToken.position).toBe(0);
        expect.int(thirdToken.length).toBe(1);
        prerr_endline("!!! END");
      });
      test("nested begin/end", ({expect, _}) => {
        prerr_endline("!!! BEGIN");
        let (tokens, _) = TextMateGrammar.tokenize(~grammar, "((a))");
        List.iter(
          t => prerr_endline(TextMateGrammar.Token.show(t)),
          tokens,
        );
        expect.int(List.length(tokens)).toBe(5);
        let firstToken = List.hd(tokens);
        expect.bool(
          firstToken.scopes
          == ["punctuation.paren.open", "expression.group", "source.abc"],
        ).
          toBe(
          true,
        );
        expect.int(firstToken.position).toBe(0);
        expect.int(firstToken.length).toBe(1);

        let secondToken = List.nth(tokens, 1);
        expect.bool(
          secondToken.scopes
          == [
               "punctuation.paren.open",
               "expression.group",
               "expression.group",
               "source.abc",
             ],
        ).
          toBe(
          true,
        );
        expect.int(secondToken.position).toBe(1);
        expect.int(secondToken.length).toBe(1);

        let thirdToken = List.nth(tokens, 2);
        expect.bool(
          thirdToken.scopes
          == [
               "keyword.letter",
               "expression.group",
               "expression.group",
               "source.abc",
             ],
        ).
          toBe(
          true,
        );
        expect.int(thirdToken.position).toBe(2);
        expect.int(thirdToken.length).toBe(1);
        prerr_endline("!!! END");
      });
      test("simple begin/end", ({expect, _}) => {
        prerr_endline("!!! BEGIN");
        let (tokens, _) = TextMateGrammar.tokenize(~grammar, "(a)");
        List.iter(
          t => prerr_endline(TextMateGrammar.Token.show(t)),
          tokens,
        );
        expect.int(List.length(tokens)).toBe(3);
        let firstToken = List.hd(tokens);
        expect.bool(
          firstToken.scopes
          == ["punctuation.paren.open", "expression.group", "source.abc"],
        ).
          toBe(
          true,
        );
        expect.int(firstToken.position).toBe(0);
        expect.int(firstToken.length).toBe(1);

        let secondToken = List.nth(tokens, 1);
        expect.bool(
          secondToken.scopes
          == ["keyword.letter", "expression.group", "source.abc"],
        ).
          toBe(
          true,
        );
        expect.int(secondToken.position).toBe(1);
        expect.int(secondToken.length).toBe(1);

        let thirdToken = List.nth(tokens, 2);
        expect.bool(
          thirdToken.scopes
          == ["punctuation.paren.close", "expression.group", "source.abc"],
        ).
          toBe(
          true,
        );
        expect.int(thirdToken.position).toBe(2);
        expect.int(thirdToken.length).toBe(1);
        prerr_endline("!!! END");
      });
    });
    test("simple letter token", ({expect, _}) => {
      let (tokens, _) = TextMateGrammar.tokenize(~grammar, "a");
      expect.int(List.length(tokens)).toBe(1);

      let firstToken = List.hd(tokens);
      expect.bool(firstToken.scopes == ["keyword.letter", "source.abc"]).toBe(
        true,
      );
      expect.int(firstToken.position).toBe(0);
      expect.int(firstToken.length).toBe(1);
    });
    test("simple word tokens", ({expect, _}) => {
      let (tokens, _) = TextMateGrammar.tokenize(~grammar, " def");
      expect.int(List.length(tokens)).toBe(1);

      let firstToken = List.hd(tokens);
      expect.bool(firstToken.scopes == ["keyword.word", "source.abc"]).toBe(
        true,
      );
      expect.int(firstToken.position).toBe(1);
      expect.int(firstToken.length).toBe(3);
    });
    test("different tokens", ({expect, _}) => {
      let (tokens, _) = TextMateGrammar.tokenize(~grammar, " adef b");
      expect.int(List.length(tokens)).toBe(3);

      let firstToken = List.hd(tokens);
      expect.bool(firstToken.scopes == ["keyword.letter", "source.abc"]).toBe(
        true,
      );
      expect.int(firstToken.position).toBe(1);
      expect.int(firstToken.length).toBe(1);

      let secondToken = List.nth(tokens, 1);
      expect.bool(secondToken.scopes == ["keyword.word", "source.abc"]).toBe(
        true,
      );
      expect.int(secondToken.position).toBe(2);
      expect.int(secondToken.length).toBe(3);

      let thirdToken = List.nth(tokens, 2);
      expect.bool(thirdToken.scopes == ["keyword.letter", "source.abc"]).toBe(
        true,
      );
      expect.int(thirdToken.position).toBe(6);
      expect.int(thirdToken.length).toBe(1);
    });
    test("capture groups", ({expect, _}) => {
      let (tokens, _) =
        TextMateGrammar.tokenize(~grammar, "@selector(windowWillClose:)");
      expect.int(List.length(tokens)).toBe(2);
      let firstToken = List.hd(tokens);
      expect.bool(firstToken.scopes == ["storage.type.objc", "source.abc"]).
        toBe(
        true,
      );
      expect.int(firstToken.position).toBe(0);
      expect.int(firstToken.length).toBe(10);

      let secondToken = List.nth(tokens, 1);
      expect.bool(secondToken.scopes == ["storage.type.objc", "source.abc"]).
        toBe(
        true,
      );
      expect.int(secondToken.position).toBe(26);
      expect.int(secondToken.length).toBe(1);
    });
    test("simple letter token", ({expect, _}) => {
      let (tokens, _) = TextMateGrammar.tokenize(~grammar, "a");
      expect.int(List.length(tokens)).toBe(1);

      let firstToken = List.hd(tokens);
      expect.bool(firstToken.scopes == ["keyword.letter", "source.abc"]).toBe(
        true,
      );
      expect.int(firstToken.position).toBe(0);
      expect.int(firstToken.length).toBe(1);
    });
    test("simple word tokens", ({expect, _}) => {
      let (tokens, _) = TextMateGrammar.tokenize(~grammar, " def");
      expect.int(List.length(tokens)).toBe(1);

      let firstToken = List.hd(tokens);
      expect.bool(firstToken.scopes == ["keyword.word", "source.abc"]).toBe(
        true,
      );
      expect.int(firstToken.position).toBe(1);
      expect.int(firstToken.length).toBe(3);
    });
    test("different tokens", ({expect, _}) => {
      let (tokens, _) = TextMateGrammar.tokenize(~grammar, " adef b");
      expect.int(List.length(tokens)).toBe(3);

      let firstToken = List.hd(tokens);
      expect.bool(firstToken.scopes == ["keyword.letter", "source.abc"]).toBe(
        true,
      );
      expect.int(firstToken.position).toBe(1);
      expect.int(firstToken.length).toBe(1);

      let secondToken = List.nth(tokens, 1);
      expect.bool(secondToken.scopes == ["keyword.word", "source.abc"]).toBe(
        true,
      );
      expect.int(secondToken.position).toBe(2);
      expect.int(secondToken.length).toBe(3);

      let thirdToken = List.nth(tokens, 2);
      expect.bool(thirdToken.scopes == ["keyword.letter", "source.abc"]).toBe(
        true,
      );
      expect.int(thirdToken.position).toBe(6);
      expect.int(thirdToken.length).toBe(1);
    });
    test("capture groups", ({expect, _}) => {
      let (tokens, _) =
        TextMateGrammar.tokenize(~grammar, "@selector(windowWillClose:)");
      expect.int(List.length(tokens)).toBe(2);
      let firstToken = List.hd(tokens);
      expect.bool(firstToken.scopes == ["storage.type.objc", "source.abc"]).
        toBe(
        true,
      );
      expect.int(firstToken.position).toBe(0);
      expect.int(firstToken.length).toBe(10);

      let secondToken = List.nth(tokens, 1);
      expect.bool(secondToken.scopes == ["storage.type.objc", "source.abc"]).
        toBe(
        true,
      );
      expect.int(secondToken.position).toBe(26);
      expect.int(secondToken.length).toBe(1);
    });
  });
});
