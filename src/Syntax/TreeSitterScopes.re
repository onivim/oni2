/*
 TreeSitterScopes
 */

open Oni_Core;
open Oni_Core_Kernel;

module Trie = Textmate.Trie;

module Matcher = {
  type t =
    // Just a single scope, like: 'string.quoted.double'
    | Scope(string)
    // A regex match clause, like: '{ match: "^http:", scopes: 'markup.underline.link'}'
    | RegExMatch(Str.regexp, string)
    // An exact match clause, like: '{ exact: "test": scopes: 'some.token' }'
    | ExactMatch(string, string);

  let getMatch = (~token="", matcher: t) => {
    switch (matcher) {
    | Scope(v) => Some(v)
    | RegExMatch(r, scope) =>
      Str.string_match(r, token, 0) ? Some(scope) : None
    | ExactMatch(m, scope) => String.equal(m, token) ? Some(scope) : None
    };
  };

  let rec firstMatch = (~token="", matchers: list(t)) => {
    switch (matchers) {
    | [] => None
    | [hd, ...tail] =>
      switch (getMatch(~token, hd)) {
      | Some(v) => Some(v)
      | None => firstMatch(~token, tail)
      }
    };
  };
};

module Selector = {
  let firstChildRegex = Str.regexp(".*:first-child$");
  let nthChildRegex = Str.regexp(".*:nth-child(\\([0-9]*\\))$");

  let checkChildSelector = (v: string) => {
    let matches = Str.string_match(nthChildRegex, v, 0);

    matches
      ? {
        let v = Str.matched_group(1, v);
        Some(v);
      }
      : Str.string_match(firstChildRegex, v, 0) ? Some("0") : None;
  };

  let removeChildSelector = (v: string) => {
    switch (String.rindex_opt(v, ':')) {
    | Some(idx) => String.sub(v, 0, idx)
    | _ => v
    };
  };

  let parse = (v: string) => {
    let childSelector = checkChildSelector(v);

    let v =
      switch (childSelector) {
      | Some(_) => removeChildSelector(v)
      | None => v
      };

    let expandedSelectors = v |> Str.split(Str.regexp(" > ")) |> List.rev;

    (expandedSelectors, childSelector);
  };
};

/*
   [TextMateConverter] is a module that helps convert
   tree-sitter paths into textmate scopes for th eming.
 */
module TextMateConverter = {
  type scopeSelector = (string, list(Matcher.t));

  type t = {
    // Tries to support 'nth-child(x)' rules
    byChildSelectors: StringMap.t(Trie.t(list(Matcher.t))),
    // Tries to support the default case - no child selector
    defaultSelectors: Trie.t(list(Matcher.t)),
  };

  let empty: t = {
    byChildSelectors: StringMap.empty,
    defaultSelectors: Trie.empty,
  };

  let create = (selectors: list(scopeSelector)) => {
    List.fold_left(
      (prev: t, curr) => {
        let {byChildSelectors, defaultSelectors} = prev;
        let (selector, matchers): scopeSelector = curr;

        let (expandedSelectors, childSelector) = Selector.parse(selector);

        let f = _ => Some(matchers);

        let (byChildSelectors, defaultSelectors) =
          switch (childSelector) {
          // If there is no child selector, just toss it in the default selectors
          | None => (
              byChildSelectors,
              Trie.update(expandedSelectors, f, defaultSelectors),
            )
          // Otherwise, we'll add an entry to the appropriate by-child selector
          | Some(childSelectorIndex) =>
            let byChildSelectors =
              StringMap.update(
                childSelectorIndex,
                v =>
                  switch (v) {
                  | None =>
                    Some(Trie.update(expandedSelectors, f, Trie.empty))
                  | Some(childSelectorTrie) =>
                    Some(
                      Trie.update(expandedSelectors, f, childSelectorTrie),
                    )
                  },
                byChildSelectors,
              );
            (byChildSelectors, defaultSelectors);
          };

        {byChildSelectors, defaultSelectors};
      },
      empty,
      selectors,
    );
  };

  let of_yojson = (json: Yojson.Safe.t) => {
    let parseSelectorList = (selectorJson: list(Yojson.Safe.t)) => {
      let f = (json: Yojson.Safe.t) => {
        switch (json) {
        | `String(v) => [Matcher.Scope(v)]
        | `Assoc(_) =>
          let exact = Yojson.Safe.Util.member("exact", json);
          let match = Yojson.Safe.Util.member("match", json);
          let scopes = Yojson.Safe.Util.member("scopes", json);

          switch (exact, match, scopes) {
          | (_, `String(m), `String(s)) => [
              Matcher.RegExMatch(Str.regexp(m), s),
            ]
          | (`String(e), _, `String(s)) => [Matcher.ExactMatch(e, s)]
          | _ => []
          };
        | _ => []
        };
      };

      selectorJson |> List.map(f) |> List.flatten;
    };

    let parseSelectors = (selectorJson: Yojson.Safe.t) => {
      switch (selectorJson) {
      | `String(v) => [Matcher.Scope(v)]
      | `List(v) => parseSelectorList(v)
      | _ => []
      };
    };

    switch (json) {
    | `Assoc(dict) =>
      let selectors =
        dict
        |> List.map(v => {
             let (key, selectorJson) = v;
             [(key, parseSelectors(selectorJson))];
           })
        |> List.flatten;
      create(selectors);
    | _ => empty
    };
  };

  let _pathToStrings = (paths: list((int, string))) => {
    List.map(snd, paths);
  };

  let _getTextMateScopeForNonChildSelector = (token, path, v) => {
    switch (Trie.matches(v.defaultSelectors, _pathToStrings(path))) {
    | [] => None
    | [hd, ..._] =>
      let (_, matchers) = hd;

      switch (matchers) {
      | None => None
      | Some(v) => Matcher.firstMatch(~token, v)
      };
    };
  };

  let getTextMateScope = (~token="", ~path=[], v: t) => {
    let check = paths => {
      let (index, _) = List.hd(paths);
      // First, try and see if a child selector matches
      let matches =
        switch (StringMap.find_opt(string_of_int(index), v.byChildSelectors)) {
        | Some(trie) =>
          switch (Trie.matches(trie, _pathToStrings(paths))) {
          | [] => None
          | [hd, ..._] =>
            let (_, matchers) = hd;

            switch (matchers) {
            | None => None
            | Some(v) => Matcher.firstMatch(~token, v)
            };
          }
        | None => None
        };

      switch (matches) {
      // If not... fall back to the default trie without child-selectors!
      | None => _getTextMateScopeForNonChildSelector(token, paths, v)
      | Some(v) => Some(v)
      };
    };

    let paths = [(0, token), ...path];

    let rec f = (p, lists) => {
      switch (p) {
      | [] => lists
      | [hd, ...tail] =>
        switch (check([hd, ...tail])) {
        | None => f(tail, lists)
        | Some(v) => f(tail, [v, ...lists])
        }
      };
    };

    let resolvedScopes = f(paths, []);
    String.concat(" ", resolvedScopes);
  };
};
