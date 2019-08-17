/*
 TreeSitterScopes
 */

open Oni_Core;

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
};

/*
   [TextMateConverter] is a module that helps convert
   tree-sitter paths into textmate scopes for th eming.
 */
module TextMateConverter = {
  type scopeSelector = (string, list(Matcher.t));

  type t = {
    // Tries to support 'nth-child(x)' rules
    byChildSelectors: IntMap.t(Trie.t(scopeSelector)),
    // Tries to support the default case - no child selector
    defaultSelectors: Trie.t(list(Matcher.t)),
  };

  let empty: t = {
    byChildSelectors: IntMap.empty,
    defaultSelectors: Trie.empty,
  };

  let create = (selectors: list(scopeSelector)) => {

    

      List.fold_left(
        (prev: t, curr) => {
          let {byChildSelectors, defaultSelectors} = prev;
          let (selector, matchers): scopeSelector = curr;
          
          let expandedSelectors =
            Str.split(Str.regexp(" > "), selector) |> List.rev;

          let f = _ => Some(matchers);

          let (byChildSelectors, defaultSelectors) = switch (Selector.checkChildSelector(selector)) {
          | None => (byChildSelectors, Trie.update(expandedSelectors, f, defaultSelectors))
          | Some(_) => (byChildSelectors, defaultSelectors);
          };
         /* | Some(v) => (byChildSelectors, defaultSelectors)
         | Some(_) => {
              IntMap.update(v, (oldKey) => switch(oldKey) {
              | None => Some(Trie.update(expandedSelectors, f, Trie.empty));
              | Some(v) => Some(Trie.update(expandedSelectors, f, v));
              }, v);
          }
          }*/

          { byChildSelectors, defaultSelectors };
        },
        empty,
        selectors,
      );
  };

  let getTextMateScope = (~index=0, ~token="", ~path=[], v: t) => {
    ignore(index);
    switch (Trie.matches(v.defaultSelectors, path)) {
    | [] => None
    | [hd, ..._] =>
      let (_, matchers) = hd;

      switch (matchers) {
      | None => None
      | Some(v) => Matcher.firstMatch(~token, v)
      };
    };
  };
};
