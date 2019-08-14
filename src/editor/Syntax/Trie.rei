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
let show: (('a) => string, t('a)) => string;

/*
  [matches(trie, path)] returns all the nodes along the path in a list of the form [(prefix, payload)]
*/
let matches: (t('a), list(string)) => list((string, option('a)));
