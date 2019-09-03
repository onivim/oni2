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

  let ofToplevelScope = (scopeName) => {
    [{ ruleName: None, scopeName, line: -1 }]
  };
};

module Token = {
  type t = {
    position: int,
    scopes: list(string),
  }

  let create = (
    ~position,
    ~scope: string,
    ~scopeStack: ScopeStack.t,
    ()
  ) => {
    let scopeNames = List.map((s: ScopeStack.scope) => s.scopeName, scopeStack);

    let ret: t = {
    position: position,
    scopes: [scope, ...scopeNames]
    };
    ret;
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
} and matchRange = {
    beginRegex: result(OnigRegExp.t, string),
    endRegex: result(OnigRegExp.t, string),
    beginCaptures: list(Capture.t),
    endCaptures: list(Capture.t),
    matchRangeName: string,
    patterns: list(pattern)
};

type t = {
  initialScopeStack: ScopeStack.t,
  scopeName: string,
  patterns: list(pattern),
  repository: StringMap.t(list(pattern)),
}

let create = (
  ~scopeName: string,
  ~patterns: list(pattern),
  ~repository: list((string, list(pattern))),
  ()) => {
  let repositoryMap = List.fold_left((prev, curr) => {
    let (scope, patterns) = curr;
    StringMap.add("#" ++ scope, patterns, prev); 
  }, StringMap.empty, repository);
  
  let ret: t = {
    initialScopeStack: ScopeStack.ofToplevelScope(scopeName),
    scopeName,
    patterns,
    repository: repositoryMap,
  };
  ret;
};

let tokenize = (~lineNumber=0, ~scopes=None, ~grammar: t, line: string) => {
  ignore(lineNumber);
  ignore(scopes);
  ignore(line);
  ([], grammar.initialScopeStack);
};
