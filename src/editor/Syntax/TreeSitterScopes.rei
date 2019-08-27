/*
 TreeSitterScopes.rei

 Module to help in mapping the tree-sitter syntax tree to textmate scopes
 */

module Matcher: {
  type t =
    // Just a single scope, like: 'string.quoted.double'
    | Scope(string)
    // A regex match clause, like: '{ match: "^http:", scopes: 'markup.underline.link'}'
    | RegExMatch(Str.regexp, string)
    // An exact match clause, like: '{ exact: "test": scopes: 'some.token' }'
    | ExactMatch(string, string);
};

module Selector: {
  let checkChildSelector: string => option(string);

  let parse: string => (list(string), option(string));
};

/*
   [TextMateConverter] is a module that helps convert
   tree-sitter paths into textmate scopes for th eming.
 */
module TextMateConverter: {
  type scopeSelector = (string, list(Matcher.t));

  type t;

  let empty: t;

  /*
     [create(scopeSelectors)] creates a converter based on a set of selectors
   */
  let create: list(scopeSelector) => t;

  let of_yojson: Yojson.Safe.json => t;

  /*
     [getTextMateScope(~index, ~token, ~path, v)] resolves information from
     the tree-sitter syntax tree into a textmate scope. The information that
     must be provided is:
     - [index] - the 'child-index' of the element, used for resolving :nth-child selectors
     - [token] - the token of the element, used in some selectors for matching
     - [path] - the path of the element from the parent, like ["pair", "value"]

     If there is no match, [None] will be returned, otherwise [Some(string)] will be returned
     with the resolved textmate scope
   */
  let getTextMateScope:
    (~index: int=?, ~token: string=?, ~path: list(string)=?, t) =>
    string;
};
