/*
 TextMateGrammar.re
 */

open Oni_Core;

open Oniguruma;

module Capture = {
  type t = (int, string);
};

module ScopeStack = {
  type scope = {
    ruleName: option(string),
    scopeName: string,
    line: int,
  };

  type t = list(scope);

  let empty: t = [];

  let ofToplevelScope = scopeName => {
    [{ruleName: None, scopeName, line: (-1)}];
  };

  let activeRule = (v: t) => {
    switch (v) {
    | [hd, ..._] => hd.ruleName
    | [] => None
    };
  };

  let pop = (v: t) => {
    switch (v) {
    | [] => v
    | [_, ...tail] => tail
    };
  };

  let push = (~ruleName: string, ~scopeName: string, ~line: int, v: t) => {
    [{ruleName: Some(ruleName), scopeName, line}, ...v];
  };
};

type pattern =
  | Include(string)
  | Match(match)
  | MatchRange(matchRange)
and match = {
  matchRegex: result(OnigRegExp.t, string),
  matchName: string,
  captures: list(Capture.t),
}
and matchRange = {
  beginRegex: result(OnigRegExp.t, string),
  endRegex: result(OnigRegExp.t, string),
  beginCaptures: list(Capture.t),
  endCaptures: list(Capture.t),
  // The scope to append to the tokens
  matchScopeName: string,
  // The rule to use when the capture group is on the top of the stack
  matchRuleName: string,
  patterns: list(pattern),
};

type t = {
  initialScopeStack: ScopeStack.t,
  scopeName: string,
  patterns: list(pattern),
  repository: StringMap.t(list(pattern)),
};

let getScope = (scope: string, v: t) =>
  StringMap.find_opt(scope, v.repository);

let getFirstRangeScope = (scope: string, v: t) => {
  switch (getScope(scope, v)) {
  | Some([MatchRange(matchRange), ..._]) => Some(matchRange)
  | _ => None
  };
};

let create =
    (
      ~scopeName: string,
      ~patterns: list(pattern),
      ~repository: list((string, list(pattern))),
      (),
    ) => {
  let repositoryMap =
    List.fold_left(
      (prev, curr) => {
        let (scope, patterns) = curr;
        StringMap.add("#" ++ scope, patterns, prev);
      },
      StringMap.empty,
      repository,
    );

  let ret: t = {
    initialScopeStack: ScopeStack.ofToplevelScope(scopeName),
    scopeName,
    patterns,
    repository: repositoryMap,
  };
  ret;
};

module Rule = {
  type t = {
    regex: OnigRegExp.t,
    name: string,
    captures: list(Capture.t),
    popStack: bool,
    pushStack: option((string, string)),
  };

  let show = (v: t) => {
    "Rule " ++ v.name;
  };

  let ofMatch = (match: match) => {
    switch (match.matchRegex) {
    | Error(_) => None
    | Ok(v) =>
      Some({
        regex: v,
        name: match.matchName,
        captures: match.captures,
        popStack: false,
        pushStack: None,
      })
    };
  };

  let ofMatchRangeBegin = (matchRange: matchRange) => {
    switch (matchRange.beginRegex) {
    | Error(_) => None
    | Ok(v) =>
      Some({
        regex: v,
        name: matchRange.matchScopeName,
        captures: matchRange.beginCaptures,
        popStack: false,
        pushStack:
          Some((matchRange.matchScopeName, matchRange.matchRuleName)),
      })
    };
  };

  let ofMatchRangeEnd = (matchRange: matchRange) => {
    switch (matchRange.endRegex) {
    | Error(_) => None
    | Ok(v) =>
      Some({
        regex: v,
        name: matchRange.matchScopeName,
        captures: matchRange.endCaptures,
        popStack: true,
        pushStack: None,
      })
    };
  };

  let rec ofPatterns = (patterns, grammar, scopeStack) => {
    let f = (prev, pattern) => {
      switch (pattern) {
      | Include(inc) =>
        prerr_endline("Rule::ofPatterns - processing Include: " ++ inc);
        switch (getScope(inc, grammar)) {
        | None =>
          prerr_endline("Rule::ofPatterns - inc not found");
          prev;
        | Some(v) =>
          prerr_endline("Rule::ofPatterns - found!");
          List.concat([ofPatterns(v, grammar, scopeStack), prev]);
        };
      | Match(match) =>
        switch (ofMatch(match)) {
        | None => prev
        | Some(v) => [v, ...prev]
        }
      | MatchRange(matchRange) =>
        switch (ofMatchRangeBegin(matchRange)) {
        | None => prev
        | Some(v) => [v, ...prev]
        }
      };
    };

    let patterns = List.fold_left(f, [], patterns);

    // If there is an active 'begin'/'end' rule - we need to grab the original range too
    switch (ScopeStack.activeRule(scopeStack)) {
    | None => patterns
    | Some(v) =>
      switch (getFirstRangeScope(v, grammar)) {
      | None => patterns
      | Some(matchRange) =>
        switch (ofMatchRangeEnd(matchRange)) {
        | None => patterns
        | Some(v) =>
          prerr_endline("Got an end rule to apply!");
          [v, ...patterns];
        }
      }
    };
  };
};

module Token = {
  type t = {
    position: int,
    length: int,
    scopes: list(string),
  };

  let create =
      (~position, ~length, ~scope: string, ~scopeStack: ScopeStack.t, ()) => {
    let scopeNames =
      List.map((s: ScopeStack.scope) => s.scopeName, scopeStack);

    let ret: t = {length, position, scopes: [scope, ...scopeNames]};
    ret;
  };

  let show = (v: t) => {
    let scopes =
      List.fold_left((prev, curr) => prev ++ "." ++ curr, "", v.scopes);
    "Token("
    ++ string_of_int(v.position)
    ++ ","
    ++ string_of_int(v.position + v.length)
    ++ ":"
    ++ scopes
    ++ ")";
  };

  let ofMatch =
      (
        ~matches: array(OnigRegExp.Match.t),
        ~rule: Rule.t,
        ~scopeStack: ScopeStack.t,
        (),
      ) => {
    switch (rule.captures) {
    | [] =>
      let match = matches[0];
      [
        create(
          ~position=match.startPos,
          ~length=match.length,
          ~scope=rule.name,
          ~scopeStack,
          (),
        ),
      ];
    | v =>
      List.map(
        cap => {
          let (idx, scope) = cap;
          let match = matches[idx];
          create(
            ~position=match.startPos,
            ~length=match.length,
            ~scope,
            ~scopeStack,
            (),
          );
        },
        v,
      )
    };
  };
};

let _getPatternsToMatchAgainst = (ruleName: option(string), grammar: t) => {
  let patterns =
    switch (ruleName) {
    | None => grammar.patterns
    | Some(v) =>
      switch (getFirstRangeScope(v, grammar)) {
      | None => grammar.patterns
      | Some(matchRange) => matchRange.patterns
      }
    };

  patterns;
};

let _getBestRule = (rules: list(Rule.t), str, position) => {
  List.fold_left(
    (prev, curr: Rule.t) => {
      let matches = OnigRegExp.search(str, position, curr.regex);
      let matchPos = Array.length(matches) > 0 ? matches[0].startPos : (-1);

      switch (prev) {
      | None when matchPos == (-1) => None
      | None => Some((matchPos, matches, curr))
      | Some(v) =>
        let (oldMatchPos, _, _) = v;
        if (matchPos < oldMatchPos && matchPos >= position) {
          Some((matchPos, matches, curr));
        } else {
          Some(v);
        };
      };
    },
    None,
    rules,
  );
};

let tokenize = (~lineNumber=0, ~scopes=None, ~grammar: t, line: string) => {
  ignore(lineNumber);
  ignore(scopes);
  ignore(line);

  let idx = ref(0);
  let len = String.length(line);

  let tokens = ref([]);

  let initialScope =
    switch (scopes) {
    | None => grammar.initialScopeStack
    | Some(v) => v
    };
  let scopeStack = ref(initialScope);

  while (idx^ < len) {
    let i = idx^;

    let currentScopeStack = scopeStack^;
    let patterns =
      _getPatternsToMatchAgainst(
        ScopeStack.activeRule(currentScopeStack),
        grammar,
      );

    let rules = Rule.ofPatterns(patterns, grammar, currentScopeStack);
    let bestRule = _getBestRule(rules, line, i);

    prerr_endline("PATTERNS: " ++ string_of_int(List.length(patterns)));
    prerr_endline("RULES: " ++ string_of_int(List.length(patterns)));
    List.iter(r => prerr_endline("!!" ++ Rule.show(r) ++ "!"), rules);
    prerr_endline("---");

    switch (bestRule) {
    // No matching rule... just increment position and try again
    | None => incr(idx)
    // Got a matching rule!
    | Some(v) =>
      open Oniguruma.OnigRegExp.Match;
      let (_, matches, rule) = v;
      if (Array.length(matches) > 0) {
        switch (rule.pushStack) {
        // If there is nothing to push... nothing to worry about
        | None => ()
        | Some((scopeName, ruleName)) =>
          scopeStack :=
            ScopeStack.push(
              ~ruleName,
              ~scopeName,
              ~line=lineNumber,
              scopeStack^,
            )
        };

        tokens :=
          [
            Token.ofMatch(~matches, ~rule, ~scopeStack=scopeStack^, ()),
            ...tokens^,
          ];

        if (rule.popStack) {
          scopeStack := ScopeStack.pop(scopeStack^);
        };

        idx := matches[0].endPos;
      } else {
        incr(idx);
      };
    };
  };

  let retTokens = tokens^ |> List.rev |> List.flatten;

  let scopeStack = scopeStack^;

  (retTokens, scopeStack);
};
