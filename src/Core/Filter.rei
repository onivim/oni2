/*
 * Filter.rei
 *
 * Module to filter & rank items using various strategies.
 */

open CamomileBundled.Camomile;

type result('a) = {
  item: 'a,
  highlight: list((int, int)),
};

let rank:
  (string, ('a, ~shouldLower: bool) => string, list('a)) => list(result('a));

/*
  [fuzzyMatches(query, str)] returns [true] if each [UChar.t] in the [query] is present
  sequentially in the string [str].
 */
let fuzzyMatches: (list(UChar.t), string) => bool;
