/*

  FirstMateTests.re

  The 'first-mate' test-suite is a suite of tests that came from https://github.com/microsoft/vscode-textmate,
  which was generated from another set of tests from Atom - https://github.com/atom/first-mate
 */

open TestFramework;

module Grammar = Textmate.Grammar;
module GrammarRepository = Textmate.GrammarRepository;
module Token = Textmate.Token;
module Tokenizer = Textmate.Tokenizer;
module StringMap =
  Map.Make({
    type t = string;
    let compare = String.compare;
  });

module FirstMateTest = {
  [@deriving yojson({strict: false})]
  type expectedToken = {
    value: string,
    scopes: list(string),
  };

  [@deriving yojson({strict: false})]
  type line = {
    line: string,
    tokens: list(expectedToken),
  };

  [@deriving yojson({strict: false})]
  type t = {
    [@default None]
    grammarScopeName: option(string),
    [@default None]
    grammarPath: option(string),
    grammars: list(string),
    lines: list(line),
    desc: string,
  };

  let _loadGrammar = (rootPath: string, grammarPath: string) => {
    let json =
      Yojson.Safe.from_file(
        "test/reason-textmate/" ++ rootPath ++ "/" ++ grammarPath,
      );
    switch (Grammar.Json.of_yojson(json)) {
    | Ok(v) => v
    | Error(msg) =>
      failwith("Unable to load grammar " ++ grammarPath ++ ": " ++ msg)
    };
  };

  let _loadGrammars = (rootPath, v: t) => {
    let allGrammars =
      switch (v.grammarPath) {
      | Some(g) => [g, ...v.grammars]
      | None => v.grammars
      };

    List.fold_left(
      (prev, curr) => {
        let grammar = _loadGrammar(rootPath, curr);
        let scopeName = Grammar.getScopeName(grammar);

        StringMap.add(scopeName, grammar, prev);
      },
      StringMap.empty,
      allGrammars,
    );
  };
  let rec _validateTokens = (et, at) => {
    switch (et, at) {
    | ([eh, ..._], []) => failwith("Expected scope not in token: " ++ eh)
    | ([], [ah, ..._]) => failwith("Extra scope present: " ++ ah)
    | ([eh, ...etail], [ah, ...atail]) =>
      if (String.equal(eh, ah)) {
        prerr_endline("Matching scopes: " ++ eh ++ " | " ++ ah);
      } else {
        failwith("Scopes do NOT match: " ++ eh ++ " | " ++ ah);
      };
      _validateTokens(etail, atail);
    | ([], []) => prerr_endline("Tokens validated!")
    };
  };

  let run = (rootPath, pass, fail, v: t) => {
    ignore(fail);

    let grammarMap = _loadGrammars(rootPath, v);
    let grammarRepository =
      GrammarRepository.create((scopeName: string) => {
        StringMap.find_opt(scopeName, grammarMap)
      });
    prerr_endline("Loaded grammars!");
    pass("Grammars loaded");

    let tokenizer = Tokenizer.create(~repository=grammarRepository, ());

    let grammar =
      switch (v.grammarPath) {
      | Some(p) => _loadGrammar(rootPath, p)
      | None =>
        switch (v.grammarScopeName) {
        | Some(s) => StringMap.find(s, grammarMap)
        | None => failwith("Unable to locate grammar")
        }
      };

    let scope = Grammar.getScopeName(grammar);

    let idx = ref(0);
    let linesArray = Array.of_list(v.lines);
    let len = Array.length(linesArray);
    let scopeStack = ref(None);

    while (idx^ < len) {
      let l = linesArray[idx^];

      prerr_endline(
        "Tokenizing line: " ++ string_of_int(idx^) ++ "|" ++ l.line ++ "|",
      );
      let line = l.line ++ "\n";
      let (tokens, newScopeStack) =
        Tokenizer.tokenize(
          ~lineNumber=idx^,
          ~scopeStack=scopeStack^,
          ~scope,
          tokenizer,
          line,
        );
      List.iter(t => prerr_endline(Token.show(t)), tokens);

      let expectedTokens = l.tokens;
      let actualTokens =
        List.map(
          (token: Token.t) => {
            let pos =
              if (line.[token.position + token.length - 1] == '\n') {
                token.length - 1;
              } else {
                token.length;
              };
            let tokenValue = String.sub(line, token.position, pos);
            let tokenScopes = token.scopes;
            (tokenValue, tokenScopes);
          },
          tokens,
        );

      let actualTokens =
        if (String.length(l.line) > 0) {
          List.filter(((v, _)) => !String.equal(v, ""), actualTokens);
        } else {
          actualTokens;
        };

      let validateToken = (idx, actualToken, expectedToken) => {
        let (actualTokenValue, actualTokenScopes) = actualToken;
        let expectedValue = expectedToken.value;
        let expectedScopes = expectedToken.scopes;

        prerr_endline("- Validating token: " ++ string_of_int(idx));
        if (String.equal(expectedValue, actualTokenValue)) {
          prerr_endline("Token Text is correct: " ++ expectedValue);
        } else {
          failwith(
            "Strings do not match - actual: "
            ++ "|"
            ++ actualTokenValue
            ++ "|"
            ++ " expected: "
            ++ "|"
            ++ expectedValue
            ++ "|",
          );
        };

        _validateTokens(expectedScopes |> List.rev, actualTokenScopes);
      };

      let rec validateTokens = (idx, expectedTokens, actualTokens) => {
        switch (expectedTokens, actualTokens) {
        | ([ehd, ...etail], [ah, ...atail]) =>
          validateToken(idx, ah, ehd);
          validateTokens(idx + 1, etail, atail);
        | ([], []) => pass("Tokens validated!")
        | _ => failwith("Token mismatch")
        };
      };

      validateTokens(0, expectedTokens, actualTokens);

      scopeStack := Some(newScopeStack);

      incr(idx);
    };
  };
};

module FirstMateTestSuite = {
  [@deriving yojson({strict: false})]
  type tests = list(FirstMateTest.t);

  type t = {
    rootPath: string,
    tests,
  };

  let ofDirectory = (rootPath: string) => {
    let json =
      Yojson.Safe.from_file(
        "test/reason-textmate/" ++ rootPath ++ "/tests.json",
      );
    let tests =
      switch (tests_of_yojson(json)) {
      | Ok(v) => v
      | Error(msg) =>
        failwith(
          "Unable to load tests from rootPath: " ++ rootPath ++ ": " ++ msg,
        )
      };

    {rootPath, tests};
  };

  let run = (runTest, v: t) => {
    List.iter(
      (t: FirstMateTest.t) => {
        runTest(
          t.desc,
          (pass, fail) => {
            prerr_endline(" === RUNNING TEST: " ++ t.desc);
            FirstMateTest.run(v.rootPath, pass, fail, t);
            prerr_endline(" === DONE WITH TEST: " ++ t.desc);
          },
        )
      },
      v.tests,
    );
  };
};

let getExecutingDirectory = () => {
  Filename.dirname(Sys.argv[0]);
};

describe("FirstMate", ({test, _}) => {
  // We'll load and parse the JSON
  let firstMateTestSuite = FirstMateTestSuite.ofDirectory("first-mate");
  let onivimTestSuite = FirstMateTestSuite.ofDirectory("onivim");

  let runTest = (name, f) => {
    test(
      name,
      ({expect, _}) => {
        let fail = msg => failwith(msg);
        let pass = msg => expect.string(msg).toEqual(msg);

        f(pass, fail);
      },
    );
  };

  let _ = runTest;
  let _ = firstMateTestSuite;
  let _ = onivimTestSuite;
  ();
  FirstMateTestSuite.run(runTest, firstMateTestSuite);
  FirstMateTestSuite.run(runTest, onivimTestSuite);
});
