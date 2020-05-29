open TestFramework;

module Grammar = Textmate.Grammar;
module RegExpFactory = Textmate.RegExpFactory;
module RegExp = Textmate.RegExp;
module Token = Textmate.Token;

let createRegex = str => {
  RegExpFactory.create(str);
};

let getExecutingDirectory = () => {
  Filename.dirname(Sys.argv[0]);
};

let grammarRepository = _ => None;

describe("Grammar", ({describe, _}) => {
  /* Test case inspired by:
      https://code.visualstudio.com/api/language-extensions/syntax-highlight-guide
     */

  describe("json parsing", ({test, _}) => {
    test("json grammar", ({expect, _}) => {
      let gr = Grammar.Json.of_file(getExecutingDirectory() ++ "/json.json");
      switch (gr) {
      | Ok(grammar) =>
        expect.string(Grammar.getScopeName(grammar)).toEqual("source.json");
        let (tokens, _) =
          Grammar.tokenize(~grammarRepository, ~grammar, "[1, true]");
        List.iter(t => prerr_endline(Token.show(t)), tokens);

        let (tokens, _) =
          Grammar.tokenize(
            ~grammarRepository,
            ~grammar,
            {|{ "name": ["a", "b"]}|},
          );
        List.iter(t => prerr_endline(Token.show(t)), tokens);
      | _ => failwith("Unable to load grammar")
      };
    })
  });

  describe("xml parsing", ({test, _}) => {
    test("json grammar", ({expect, _}) => {
      let result =
        Grammar.Xml.of_file(getExecutingDirectory() ++ "/json.json");

      expect.result(result).toBeError();
    });

    test("xml grammar: haskell", ({expect, _}) => {
      let result =
        Grammar.Xml.of_file(getExecutingDirectory() ++ "/haskell.tmLanguage");

      expect.result(result).toBeOk();

      let grammar = Result.get_ok(result);
      expect.string(grammar.scopeName).toEqual("source.haskell");
    });

    test("xml grammar: lua", ({expect, _}) => {
      let result =
        Grammar.Xml.of_file(getExecutingDirectory() ++ "/lua.tmLanguage");

      expect.result(result).toBeOk();

      let grammar = Result.get_ok(result);
      expect.string(grammar.scopeName).toEqual("source.lua");
    });
  });

  let grammar =
    Grammar.create(
      ~scopeName="source.abc",
      ~patterns=[Include("source.abc", "#expression")],
      ~repository=[
        (
          "expression",
          [
            Include("source.abc", "#letter"),
            Include("source.abc", "#word"),
            Include("source.abc", "#capture-groups"),
            Include("source.abc", "#paren-expression"),
          ],
        ),
        (
          "letter",
          [
            Match({
              matchRegex: createRegex("a|b|c"),
              matchName: Some("keyword.letter"),
              captures: [],
            }),
          ],
        ),
        (
          "word",
          [
            Match({
              matchRegex: createRegex("def"),
              matchName: Some("keyword.word"),
              captures: [],
            }),
          ],
        ),
        (
          "capture-groups",
          [
            Match({
              matchRegex: createRegex("(@selector\\()(.*?)(\\))"),
              matchName: Some("capture-group"),
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
              beginRegex: createRegex("\\("),
              endRegex: createRegex("\\)"),
              beginCaptures: [(0, "punctuation.paren.open")],
              endCaptures: [(0, "punctuation.paren.close")],
              name: Some("expression.group"),
              contentName: None,
              patterns: [Include("source.abc", "#expression")],
              applyEndPatternLast: false,
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
          Grammar.tokenize(~grammarRepository, ~grammar, "(");
        let (line2Token, line2Scope) =
          Grammar.tokenize(
            ~grammarRepository,
            ~grammar,
            ~scopes=Some(line1Scope),
            "a",
          );
        let (line3Token, _) =
          Grammar.tokenize(
            ~grammarRepository,
            ~grammar,
            ~scopes=Some(line2Scope),
            ")",
          );
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
        let (tokens, _) =
          Grammar.tokenize(~grammarRepository, ~grammar, "((a))");
        List.iter(t => prerr_endline(Token.show(t)), tokens);
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
        let (tokens, _) =
          Grammar.tokenize(~grammarRepository, ~grammar, "(a)");
        List.iter(t => prerr_endline(Token.show(t)), tokens);
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
      let (tokens, _) = Grammar.tokenize(~grammarRepository, ~grammar, "a");
      expect.int(List.length(tokens)).toBe(1);

      let firstToken = List.hd(tokens);
      expect.bool(firstToken.scopes == ["keyword.letter", "source.abc"]).toBe(
        true,
      );
      expect.int(firstToken.position).toBe(0);
      expect.int(firstToken.length).toBe(1);
    });
    test("simple word tokens", ({expect, _}) => {
      let (tokens, _) =
        Grammar.tokenize(~grammarRepository, ~grammar, " def");
      expect.int(List.length(tokens)).toBe(2);

      let secondToken = List.nth(tokens, 1);
      expect.bool(secondToken.scopes == ["keyword.word", "source.abc"]).toBe(
        true,
      );
      expect.int(secondToken.position).toBe(1);
      expect.int(secondToken.length).toBe(3);
    });
    test("different tokens", ({expect, _}) => {
      let (tokens, _) =
        Grammar.tokenize(~grammarRepository, ~grammar, " adef b");
      expect.int(List.length(tokens)).toBe(5);

      let secondToken = List.nth(tokens, 1);
      expect.bool(secondToken.scopes == ["keyword.letter", "source.abc"]).
        toBe(
        true,
      );
      expect.int(secondToken.position).toBe(1);
      expect.int(secondToken.length).toBe(1);

      let thirdToken = List.nth(tokens, 2);
      expect.bool(thirdToken.scopes == ["keyword.word", "source.abc"]).toBe(
        true,
      );
      expect.int(thirdToken.position).toBe(2);
      expect.int(thirdToken.length).toBe(3);

      let fifthToken = List.nth(tokens, 4);
      expect.bool(fifthToken.scopes == ["keyword.letter", "source.abc"]).toBe(
        true,
      );
      expect.int(fifthToken.position).toBe(6);
      expect.int(fifthToken.length).toBe(1);
    });
    test("capture groups", ({expect, _}) => {
      let (tokens, _) =
        Grammar.tokenize(
          ~grammarRepository,
          ~grammar,
          "@selector(windowWillClose:)",
        );
      expect.int(List.length(tokens)).toBe(3);
      let firstToken = List.hd(tokens);
      expect.bool(
        firstToken.scopes
        == ["storage.type.objc", "capture-group", "source.abc"],
      ).
        toBe(
        true,
      );
      expect.int(firstToken.position).toBe(0);
      expect.int(firstToken.length).toBe(10);

      let thirdToken = List.nth(tokens, 2);
      expect.bool(
        thirdToken.scopes
        == ["storage.type.objc", "capture-group", "source.abc"],
      ).
        toBe(
        true,
      );
      expect.int(thirdToken.position).toBe(26);
      expect.int(thirdToken.length).toBe(1);
    });
    test("simple letter token", ({expect, _}) => {
      let (tokens, _) = Grammar.tokenize(~grammarRepository, ~grammar, "a");
      expect.int(List.length(tokens)).toBe(1);

      let firstToken = List.hd(tokens);
      expect.bool(firstToken.scopes == ["keyword.letter", "source.abc"]).toBe(
        true,
      );
      expect.int(firstToken.position).toBe(0);
      expect.int(firstToken.length).toBe(1);
    });
  });
});
