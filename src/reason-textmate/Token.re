/*
 Token.re
 */

open Oniguruma;

type t = {
  position: int,
  length: int,
  scopes: list(string),
};

let _sanitizeScopes = scopes => {
  scopes
  |> List.map(s => List.rev(String.split_on_char(' ', s)))
  |> List.flatten;
};

let create =
    (
      ~position,
      ~length,
      ~scope=None,
      ~outerScope=None,
      ~scopeStack: ScopeStack.t,
      (),
    ) => {
  let scopeNames = ScopeStack.getScopes(scopeStack);

  let scopes =
    switch (scope, outerScope) {
    | (Some(s), Some(o)) => [s, o, ...scopeNames]
    | (Some(s), None) => [s, ...scopeNames]
    | (None, Some(o)) => [o, ...scopeNames]
    | _ => scopeNames
    };

  let ret: t = {length, position, scopes: _sanitizeScopes(scopes)};
  ret;
};

let _create2 = (~position, ~length, ~scopes, ()) => {
  position,
  length,
  scopes: _sanitizeScopes(scopes),
};

let show = (v: t) => {
  let scopes =
    List.fold_left((prev, curr) => prev ++ ", " ++ curr, "", v.scopes);
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
    let name =
      switch (rule.name, rule.pushStack, rule.popStack) {
      | (Some(name), None, None) => Some(name)
      | _ => None
      };
    let match = matches[0];
    [
      create(
        ~position=match.startPos,
        ~length=match.length,
        ~scope=name,
        ~scopeStack,
        (),
      ),
    ];
  | v =>
    let initialMatch = matches[0];

    /*If the rule is a 'push stack', the outer rule has already been applied
          because the scope stack has been updated.
          If there rule is not a 'push stack', then we need to apply the rule here
          locally, for patterns like this:

      {
       "match": "world(!?)",
       "captures": {
         "1": {
           "name": "emphasis.hello"
         }
       },
       "name": "suffix.hello"
      }

            For a string like "world!", we'd expect two tokens:
            - "hello" - ["suffix.hello"]
            - "!" - ["emphasis.hello", "suffix.hello"]
          */
    let scopeNames = ScopeStack.getScopes(scopeStack);
    let initialScope =
      switch (rule.name, rule.pushStack, rule.popStack) {
      | (Some(name), None, None) => [name, ...scopeNames]
      | _ => scopeNames
      };

    // Create an array for each element in the match
    let len = initialMatch.length;
    let scopeArray = Array.make(initialMatch.length, initialScope);

    let matchesLen = Array.length(matches);
    // Apply each capture group to the array
    List.iter(
      cg => {
        let (idx, scope) = cg;

        if (idx < matchesLen) {
          let match = matches[idx];

          if (match.length > 0 && match.startPos < initialMatch.endPos) {
            let idx = ref(match.startPos - initialMatch.startPos);
            let endPos = min(len, match.endPos - initialMatch.startPos);

            while (idx^ < endPos) {
              let i = idx^;
              scopeArray[i] = [scope, ...scopeArray[i]];
              incr(idx);
            };
          };
        };
      },
      v,
    );

    let rec scopesAreEqual = (scopeList1, scopeList2) =>
      if (scopeList1 === scopeList2) {
        true;
      } else {
        switch (scopeList1, scopeList2) {
        | ([h1, ...t1], [h2, ...t2]) =>
          h1 == h2 ? scopesAreEqual(t1, t2) : false
        | ([], []) => true
        | _ => false
        };
      };

    // Iterate across array and make tokens

    let lastTokenPosition = ref(0);
    let idx = ref(1);
    let len = initialMatch.length;
    let tokens = ref([]);

    while (idx^ < len) {
      let i = idx^;
      let prev = i - 1;
      let curScopes = scopeArray[i];
      let prevScopes = scopeArray[prev];

      let ltp = lastTokenPosition^;

      if (!scopesAreEqual(curScopes, prevScopes) && i - ltp > 0) {
        tokens :=
          [
            _create2(
              ~position=ltp + initialMatch.startPos,
              ~length=i - ltp,
              ~scopes=prevScopes,
              (),
            ),
            ...tokens^,
          ];
        lastTokenPosition := i;
      };

      incr(idx);
    };

    let ltp = lastTokenPosition^;
    if (ltp < len && len - ltp > 0) {
      tokens :=
        [
          _create2(
            ~position=ltp + initialMatch.startPos,
            ~length=len - ltp,
            ~scopes=scopeArray[len - 1],
            (),
          ),
          ...tokens^,
        ];
    };

    tokens^;
  };
};
