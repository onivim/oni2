/*
 Interface for the 'colors' section of a Theme
 */
module ColorTheme: {
  type t;

  let empty: t;

  /*
   [of_yojson] instantiates a TokenTheme [t] from JSON
   */
  let of_yojson: Yojson.Safe.t => t;

  let of_map: StringMap.t(string) => t;

  let union: (t, t) => t;

  let fold: ((string, string, 'a) => 'a, t, 'a) => 'a;

  let getColor: (string, t) => option(string);

  let getFirstOrDefault: (~default: string, list(string), t) => string;
};

module Grammar: {
  type t;

  type grammarRepository = string => option(t);

  let getScopeName: t => string;

  let create:
    (
      ~scopeName: string,
      ~patterns: list(Pattern.t),
      ~repository: list((string, list(Pattern.t))),
      unit
    ) =>
    t;

  module Json: {
    let of_yojson: Yojson.Safe.t => result(t, string);
    let of_file: string => result(t, string);
  };

  module Xml: {let of_file: string => result(t, string);};

  let tokenize:
    (
      ~lineNumber: int=?,
      ~scopes: option(ScopeStack.t)=?,
      ~grammarRepository: grammarRepository,
      ~grammar: t,
      string
    ) =>
    (list(Token.t), ScopeStack.t);
};

module GrammarRepository: {
  type t;

  let getGrammar: (t, string) => option(Grammar.t);

  let ofGrammar: (string, Grammar.t) => t;
  let ofFilePath: (string, string) => t;

  type grammarRepository = string => option(Grammar.t);

  let create: grammarRepository => t;
};

/*
 Interface for interacting with VSCode-flavored TextMate themes
 */
module Theme: {
  type t;

  type themeLoader = string => result(t, string);

  let of_yojson:
    (~isDark: bool=?, ~themeLoader: themeLoader, Yojson.Safe.t) => t;
  let from_file: (~isDark: bool=?, string) => result(t, string);

  let getColors: t => ColorTheme.t;
  let getTokenColors: t => TokenTheme.t;

  let isDark: t => bool;
};

module TokenTheme: {
  /*
   TokenTheme.rei

   Interface for textmate theme matching
   */

  open ThemeScopes;

  /*
     [themeSelector] describes a [string] selector,
     along with the [TokenStyle.t] styling associated with that selector.
   */
  type themeSelector = (string, TokenStyle.t);

  /*
     [t] is a description of a TextMate theme
   */
  type t;

  /*
     [create] builds a TokenTheme [t] from a list of styles
   */
  let create:
    (
      ~defaultBackground: string,
      ~defaultForeground: string,
      list(themeSelector)
    ) =>
    t;

  /*
      [of_yojson] instantiates a TokenTheme [t] from JSON
   */
  let of_yojson:
    (~defaultBackground: string, ~defaultForeground: string, Yojson.Safe.t) =>
    t;

  /*
     [empty] is an empty TokenTheme [t] with no selectors
   */
  let empty: t;

  let union:
    (~defaultBackground: string, ~defaultForeground: string, t, t) => t;

  /*
      [match] returns the resolved style information,
      given the scopes [string]. The [scopes] should include
      the full ancestor list, separated by spaces, for example:
      "text.html.basic source.php string.quoted.double.php"

      Returns styling information based on the selecotrs.
   */
  let match: (t, string) => ResolvedStyle.t;

  /*
     [show] returns a string representation of the TokenTheme [t]
   */
  let show: t => string;
};

module Tokenizer: {
  /*
   Tokenizer.rei
   */

  type t;

  let create: (~repository: GrammarRepository.t, unit) => t;

  let tokenize:
    (
      ~lineNumber: int=?,
      ~scopeStack: option(ScopeStack.t)=?,
      ~scope: string,
      t,
      string
    ) =>
    (list(Token.t), ScopeStack.t);
};

module Trie: {
  /*
   Trie.rei

   Simple Trie implementation to support scope selection
   */

  /*
     [t] represents the Trie with a type parameter ['a]
   */
  type t('a);

  /*
     [empty] is the empty Trie
   */
  let empty: t('a);

  /*
     [update] is used to add or modify nodes to the Trie, and takes the following:
     - path [list(string)] - the path to the node to add or modify
     - f [option('a) => option('a)] - a function that is given the existing node and returns a new node

     [update] returns a new Trie, with the function [f] applied to the node. Empty nodes with [None]
     are created along the route if they do not already exist.
   */
  let update: (list(string), option('a) => option('a), t('a)) => t('a);

  /*
    [show(printer, trie)] gives a [string] representation of a trie [t('a)],
    given a [printer] function ['a => string];
   */
  let show: ('a => string, t('a)) => string;

  /*
     [matches(trie, path)] returns all the nodes along the path in a list of the form [(prefix, payload)]
   */
  let matches: (t('a), list(string)) => list((string, option('a)));
};
