/*
 TextMateGrammar.re
 */

type t = {
  initialScopeStack: ScopeStack.t,
  scopeName: string,
  patterns: list(Pattern.t),
  repository: StringMap.t(list(Pattern.t)),
};

type grammarRepository = string => option(t);

let getScopeName = (v: t) => v.scopeName;

let getScope = (repo: grammarRepository, grammarScope, scope: string, v: t) => {
  let len = String.length(scope);

  let grammar =
    if (String.equal(grammarScope, v.scopeName)) {
      Some(v);
    } else {
      repo(grammarScope);
    };

  switch (grammar) {
  | Some(g) =>
    if (len > 0 && scope.[0] == '#') {
      StringMap.find_opt(scope, g.repository);
    } else {
      // This is implicity '$self'
      Some(g.patterns);
    }
  | None => None
  };
};

let create =
    (
      ~scopeName: string,
      ~patterns: list(Pattern.t),
      ~repository: list((string, list(Pattern.t))),
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
    initialScopeStack: ScopeStack.ofTopLevelScope(patterns, scopeName),
    scopeName,
    patterns,
    repository: repositoryMap,
  };
  ret;
};

module Json = {
  open Yojson.Safe.Util;

  let patterns_of_yojson = (scopeName, json: Yojson.Safe.t) => {
    switch (json) {
    | `List(v) =>
      List.fold_left(
        (prev, curr) => {
          switch (prev) {
          | Error(e) => Error(e)
          | Ok(currItems) =>
            switch (Pattern.Json.of_yojson(scopeName, curr)) {
            | Error(e) => Error(e)
            | Ok(v) => Ok([v, ...currItems])
            }
          }
        },
        Ok([]),
        v,
      )
    | _ => Error("Patterns is expected to be a list")
    };
  };

  let repository_of_yojson = (scope, json: Yojson.Safe.t) => {
    switch (json) {
    | `Assoc(v) =>
      List.fold_left(
        (prev, curr) => {
          switch (prev) {
          | Error(e) => Error(e)
          | Ok(currItems) =>
            let (key, json) = curr;

            // Is this a nested set of patterns?
            switch (member("begin", json), member("patterns", json)) {
            // Yes...
            | (`Null, `List(_) as patternList) =>
              let patterns = patterns_of_yojson(scope, patternList);
              switch (patterns) {
              | Error(e) => Error(e)
              | Ok(v) => Ok([(key, v), ...currItems])
              };
            // Nope... just a single pattern
            | _ =>
              switch (Pattern.Json.of_yojson(scope, json)) {
              | Error(e) => Error(e)
              | Ok(v) => Ok([(key, [v]), ...currItems])
              }
            };
          }
        },
        Ok([]),
        v,
      )
    | _ => Ok([])
    };
  };

  let string_of_yojson: Yojson.Safe.t => result(string, string) =
    json => {
      switch (json) {
      | `String(v) => Ok(v)
      | _ => Error("Missing expected property")
      };
    };

  let of_yojson = (json: Yojson.Safe.t) => {
    let%bind scopeName = string_of_yojson(member("scopeName", json));
    let%bind patterns =
      patterns_of_yojson(scopeName, member("patterns", json));
    let%bind repository =
      repository_of_yojson(scopeName, member("repository", json));

    Ok(create(~scopeName, ~patterns, ~repository, ()));
  };

  let of_file = (path: string) => {
    Yojson.Safe.from_file(path) |> of_yojson;
  };
};

module Xml = {
  module PlistDecoder = {
    open Plist;

    let repository = scopeName =>
      assoc(
        oneOf([
          ("pattern", Pattern.of_plist(scopeName) |> map(pat => [pat])),
          (
            "patterns",
            property("patterns", array(Pattern.of_plist(scopeName))),
          ),
        ]),
      );

    let grammar = plist => {
      let%bind scopeName = Plist.property("scopeName", Plist.string, plist);

      dict(
        prop =>
          create(
            ~scopeName,
            ~patterns=
              prop.required("patterns", array(Pattern.of_plist(scopeName))),
            ~repository=
              prop.withDefault("repository", repository(scopeName), []),
            (),
          ),
        plist,
      );
    };
  };

  let of_file = path => {
    let%bind plist =
      SimpleXml.of_file(path) |> Option.get |> XmlPlistParser.parse;

    PlistDecoder.grammar(plist);
  };
};

let _getBestRule = (lastMatchedRange, rules: list(Rule.t), str, position) => {
  let rules =
    switch (lastMatchedRange) {
    // Filter out any rule that 'pushes' or 'pops' with the same pattern we
    // had before, if we're at the same position. This prevents infinite loops,
    // where a pattern might have a non-consuming match.
    | Some((pos, matchRange)) when pos == position =>
      let filter = (rule: Rule.t) =>
        switch (rule.popStack, rule.pushStack) {
        | (Some(mr), _) when mr === matchRange => false
        | (_, Some(mr)) when mr === matchRange => false
        | _ => true
        };
      List.filter(filter, rules);
    | _ => rules
    };

  let rec checkRule =
          (
            prev: option((int, array(Oniguruma.OnigRegExp.Match.t), Rule.t)),
            rules: list(Rule.t),
          ) => {
    switch (rules) {
    | [] => prev
    | [hd, ...tail] =>
      let regex = hd.regex;
      let matchPos = RegExp.search(str, position, regex);

      switch (prev) {
      // Case 1: No match, but we have a match at the checked position -> apply rule
      | None when matchPos == position =>
        Some((matchPos, RegExp.matches(regex), hd))
      // Case 2: No current match, and no new match -> check next rules
      | None when matchPos == (-1) => checkRule(None, tail)
      // Case 3: We have a match, and we didn't have one before, but we don't know it is the best one -> compare with next rules
      | None => checkRule(Some((matchPos, RegExp.matches(regex), hd)), tail)
      // Case 4: We had a previous match, but we now have a match that matches the current position -> use this match
      | Some(_) when matchPos == position =>
        Some((matchPos, RegExp.matches(regex), hd))
      // Case 5: We had a previous match, and a new match, but there still could be a better rule.
      // Pick the best one out of the prev / new, and look for new matches.
      | Some(v) =>
        let (oldMatchPos, _, _) = v;
        if (matchPos < oldMatchPos && matchPos >= position) {
          checkRule(Some((matchPos, RegExp.matches(regex), hd)), tail);
        } else {
          checkRule(Some(v), tail);
        };
      };
    };
  };

  checkRule(None, rules);
};

let tokenize =
    (
      ~lineNumber=0,
      ~scopes=None,
      ~grammarRepository,
      ~grammar: t,
      line: string,
    ) => {
  let idx = ref(0);
  let lastTokenPosition = ref(0);
  let lastAnchorPosition = ref(-1);
  let len = String.length(line);
  let lastMatchedRange = ref(None);

  let tokens = ref([]);

  let initialScope =
    switch (scopes) {
    | None => grammar.initialScopeStack
    | Some(v) => v
    };

  let scopeStack = ref(initialScope);

  // Iterate across the string and tokenize
  while (idx^ <= len) {
    let i = idx^;

    let currentScopeStack = scopeStack^;

    // Get active set of patterns...
    let patterns = ScopeStack.activePatterns(currentScopeStack);

    // ...and then get rules from the patterns.
    let rules =
      Rule.ofPatterns(
        ~isFirstLine=lineNumber == 0,
        ~isAnchorPos=lastAnchorPosition^ == i,
        ~getScope=
          (scope, inc) => getScope(grammarRepository, scope, inc, grammar),
        ~scopeStack=currentScopeStack,
        patterns,
      );

    // And figure out if any of the rules applies.
    let bestRule = _getBestRule(lastMatchedRange^, rules, line, i);

    switch (bestRule) {
    // No matching rule... just increment position and try again
    | None => incr(idx)
    // Got a matching rule!
    | Some(v) =>
      open Oniguruma.OnigRegExp.Match;
      let (_, matches, rule) = v;
      let ltp = lastTokenPosition^;
      // Logging around rule evaluation

      // print_endline ("Last anchor position: " ++ string_of_int(lastAnchorPosition^));
      // print_endline ("Matching rule: " ++ Rule.show(rule));

      // If we skipped a bunch of characters, we need to add a token for it.
      if (ltp < matches[0].startPos) {
        let newToken =
          Token.create(
            ~position=ltp,
            ~length=matches[0].startPos - ltp,
            ~scopeStack=scopeStack^,
            (),
          );
        lastTokenPosition := matches[0].startPos;

        // Logging around token creation
        /* print_endline ("Match - startPos: "
            ++ string_of_int(matches[0].startPos)
            ++ "endPos: " ++ string_of_int(matches[0].endPos));
           print_endline("Creating token at " ++ string_of_int(ltp) ++ ":" ++ Token.show(newToken));*/

        let prevToken = [newToken];
        tokens := [prevToken, ...tokens^];
      };

      // Check if the rule pushes onto the scope stack, and handle it!
      switch (rule.pushStack) {
      | None => ()
      | Some(matchRange) =>
        scopeStack :=
          ScopeStack.pushPattern(
            ~matches,
            ~matchRange,
            ~line=lineNumber,
            scopeStack^,
          );

        switch (matchRange.name) {
        | None => ()
        | Some(n) => scopeStack := ScopeStack.pushScope(n, scopeStack^)
        };
      };

      switch (rule.popStack) {
      | None => ()
      | Some(mr) =>
        switch (mr.contentName) {
        | None => ()
        | Some(_) => scopeStack := ScopeStack.popScope(scopeStack^)
        }
      };

      // If there was a match, and it is non-zero-length, we'll create a token for it.
      if (matches[0].endPos > matches[0].startPos) {
        tokens :=
          [
            Token.ofMatch(~matches, ~rule, ~scopeStack=scopeStack^, ()),
            ...tokens^,
          ];
        lastTokenPosition := matches[0].endPos;
      };

      switch (rule.pushStack) {
      | None => ()
      | Some(matchRange) =>
        switch (matchRange.contentName) {
        | None => ()
        | Some(n) => scopeStack := ScopeStack.pushScope(n, scopeStack^)
        }
      };

      switch (rule.popStack) {
      | None => ()
      | Some(mr) =>
        scopeStack := ScopeStack.popPattern(scopeStack^);
        switch (mr.name) {
        | None => ()
        | Some(_) => scopeStack := ScopeStack.popScope(scopeStack^)
        };
      };

      let prevIndex = idx^;
      idx := max(matches[0].endPos, prevIndex);

      let pos = idx^;
      switch (rule.popStack, rule.pushStack) {
      // If the rule isn't a push or pop rule, and we're at the same index, we're stuck
      // in a loop - we'll push forward a character in that case.
      | (None, None) when pos <= prevIndex => incr(idx)
      // Otherwise, if it's a push rule, record that we pushed so that we can break an infinite loop
      | (None, Some(mr)) =>
        lastMatchedRange := Some((prevIndex, mr));
        lastAnchorPosition := matches[0].endPos;
      | _ => ()
      };
    };
  };

  // There might be some leftover whitespace or tokens
  // that weren't processed through our loop iteration.
  let tokens =
    if (len == 0) {
      [[Token.create(~position=0, ~length=0, ~scopeStack=scopeStack^, ())]];
    } else if (lastTokenPosition^ < len) {
      [
        [
          Token.create(
            ~position=lastTokenPosition^,
            ~length=len - lastTokenPosition^,
            ~scopeStack=scopeStack^,
            (),
          ),
        ],
        ...tokens^,
      ];
    } else {
      tokens^;
    };

  let retTokens = tokens |> List.flatten |> List.rev;

  let scopeStack = scopeStack^;

  (retTokens, scopeStack);
};
