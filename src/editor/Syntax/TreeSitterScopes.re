/*
 TreeSitterScopes
 */

module Matcher  = {
  
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
    | RegExMatch(r, scope) => Str.string_match(r, token, 0) ? Some(scope) : None
    | ExactMatch(m, scope) => String.equal(m, token) ? Some(scope) : None
    };
  };

  let rec firstMatch = (~token="", matchers: list(t)) => {
    switch (matchers) {
    | [] => None
    | [hd, ...tail] => {
      switch (getMatch(~token, hd)) {
      | Some(v) => Some(v)
      | None => firstMatch(~token, tail)
      }
    } 
    }
  };
  
}

/*
  [TextMateConverter] is a module that helps convert
  tree-sitter paths into textmate scopes for th eming.
*/
module TextMateConverter = {
  type scopeSelector = (string, list(Matcher.t));

  type t = {
    // Tries to support 'nth-child(x)' rules
    // byChildSelectors: IntMap.t(Trie.t(scopeSelector)),
    // Tries to support the default case - no child selector
    defaultSelectors: Trie.t(list(Matcher.t)),
  };

  let create = (selectors: list(scopeSelector)) => {
    let defaultSelectors = List.fold_left(
      (prev, curr) => {

        let (selector, matchers): scopeSelector = curr;
        let expandedSelectors = Str.split(Str.regexp(" > "), selector) |> List.rev;
        let f = _ => Some(matchers);
        Trie.update(expandedSelectors, f, prev);
      }, Trie.empty, selectors);

      let ret: t = { 
        defaultSelectors: defaultSelectors,
      };
      ret;
  };

  let getTextMateScope = (~_index=0, ~token="", ~path=[], v: t) => {
    switch (Trie.matches(v.defaultSelectors, path)) {
    | [] => None
    | [hd, ..._] => {

      let (_, matchers) = hd;

      switch (matchers) {
      | None => None
      | Some(v) => Matcher.firstMatch(~token, v);
      }

    }
    }
  };
};

