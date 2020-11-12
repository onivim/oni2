/*
 Rule.re
 */

type t = {
  regex: RegExp.t,
  name: option(string),
  captures: list(Pattern.Capture.t),
  popStack: option(Pattern.matchRange),
  pushStack: option(Pattern.matchRange),
};

let show = (v: t) => {
  let start =
    switch (v.name) {
    | Some(rule) => "Rule " ++ rule ++ ": "
    | None => "Rule (anonymous): "
    };

  start ++ RegExp.toString(v.regex);
};

let ofMatch = (allowA, allowG, match: Pattern.patternMatch) => {
  Some({
    regex: RegExpFactory.compile(allowA, allowG, match.matchRegex),
    name: match.matchName,
    captures: match.captures,
    popStack: None,
    pushStack: None,
  });
};

let ofMatchRangeBegin = (allowA, allowG, matchRange: Pattern.matchRange) => {
  Some({
    regex: RegExpFactory.compile(allowA, allowG, matchRange.beginRegex),
    name: matchRange.name,
    captures: matchRange.beginCaptures,
    popStack: None,
    pushStack: Some(matchRange),
  });
};

let ofMatchRangeEnd = (allowA, allowG, matchRange: Pattern.matchRange) => {
  regex: RegExpFactory.compile(allowA, allowG, matchRange.endRegex),
  name: matchRange.name,
  captures: matchRange.endCaptures,
  popStack: Some(matchRange),
  pushStack: None,
};

let ofPatterns =
    (
      ~isFirstLine,
      ~isAnchorPos,
      ~getScope,
      ~scopeStack,
      patterns: list(Pattern.t),
    ) => {
  let rec f = (prev, pattern) => {
    switch (pattern) {
    | Pattern.Include(scope, inc) =>
      switch (getScope(scope, inc)) {
      | None => prev
      | Some(v) => List.fold_left(f, prev, v)
      }
    | Pattern.Match(match) =>
      switch (ofMatch(isFirstLine, isAnchorPos, match)) {
      | None => prev
      | Some(v) => [v, ...prev]
      }
    | Pattern.MatchRange(matchRange) =>
      switch (ofMatchRangeBegin(isFirstLine, isAnchorPos, matchRange)) {
      | None => prev
      | Some(v) => [v, ...prev]
      }
    };
  };

  let activeRange = ScopeStack.activeRange(scopeStack);

  let initialRules =
    switch (activeRange) {
    | Some(v) when v.applyEndPatternLast == true => [
        ofMatchRangeEnd(isFirstLine, isAnchorPos, v),
      ]
    | _ => []
    };

  let rules = List.fold_left(f, initialRules, patterns);

  switch (activeRange) {
  | Some(v) when v.applyEndPatternLast == false => [
      ofMatchRangeEnd(isFirstLine, isAnchorPos, v),
      ...rules,
    ]
  | _ => rules
  };
};
