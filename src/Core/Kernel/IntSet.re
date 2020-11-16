/*
 * IntSet.re
 *
 * Set of integers
 */

include Set.Make({
  type t = int;
  let compare = compare;
});
