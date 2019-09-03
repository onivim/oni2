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
    ruleName: string,
    scopeName: string,
    line: int,
  };

  type t = list(scope);

  let empty: t = [];
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
    let scopeNames = List.map((s) => s.scopeName, scopeStack);

    {
    position,
    scopes: [scope, ...scopeStack]
    };
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
    scopeName,
    patterns,
    repository: repositoryMap,
  };
  ret;
};

let tokenizeLine = (~lineNumber=0, ~scopes=ScopeStack.empty, ~grammar: t, line: string) => {
  ignore(lineNumber);
  ignore(scopes);
  ignore(grammar);
  ([], scopes);
};
