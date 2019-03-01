/*
 * IntMap.re
 *
 * Map from int -> 'a
 */

include Map.Make({
  type t = int;
  let compare = compare;
});
