/*
 * Filter.rei
 *
 * Module to filter & rank items using various strategies.
 */

type result('a) = FilterJob.rankResult('a);
let rank: (string, ('a, ~shouldLower:bool) => string, list('a)) => list(result('a));