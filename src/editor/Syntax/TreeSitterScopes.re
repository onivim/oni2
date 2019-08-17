/*
 TreeSitterScopes
 */

/*
  [TextMateConverter] is a module that helps convert
  tree-sitter paths into textmate scopes for th eming.
*/
module TextMateConverter = {
  type matcher =
  // Just a single scope, like: 'string.quoted.double'
  | Scope(string)
  // A regex match clause, like: '{ match: "^http:", scopes: 'markup.underline.link'}'
  | RegExMatch(string, string)
  // An exact match clause, like: '{ exact: "test": scopes: 'some.token' }'
  | ExactMatch(string, string);

  type scopeSelector = (string, list(matcher));

  type t = {
    // Tries to support 'nth-child(x)' rules
    // byChildSelectors: IntMap.t(Trie.t(scopeSelector)),
    // Tries to support the default case - no child selector
    defaultSelectors: Trie.t(list(matcher)),
  };

  let create = (selectors: list(scopeSelector)) => {
    let defaultSelectors = List.fold_left(
      (prev, curr) => {
        let (selector, matchers): scopeSelector = curr;
        let f = _ => Some(matchers);
        Trie.update([selector], f, prev);
      }, Trie.empty, selectors);

      { defaultSelectors };
  };
};

