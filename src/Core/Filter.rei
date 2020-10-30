/*
 * Filter.rei
 *
 * Module to filter & rank items using various strategies.
 */

type result('a) = {
  item: 'a,
  highlight: list((int, int)),
  score: float,
};

let result: 'a => result('a);

let map: ('a => 'b, result('a)) => result('b);

let rank:
  (string, ('a, ~shouldLower: bool) => string, list('a)) => list(result('a));

/*
  [fuzzyMatches(query, str)] returns [true] if each [UChar.t] in the [query] is present
  sequentially in the string [str].
 */
let fuzzyMatches: (list(Uchar.t), string) => bool;
