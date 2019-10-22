/*
 * Filter.rei
 *
 * Module to filter & rank items using various strategies.
 */

type result('a) = {
  item: 'a,
  highlight: list((int, int))
}

let rank: (string, ('a, ~shouldLower:bool) => string, list('a)) => list(result('a));