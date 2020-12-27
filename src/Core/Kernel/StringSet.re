/*
 * StringMap.re
 *
 * Map from string -> 'a
 */

include Set.Make({
  type t = string;
  let compare = String.compare;
});
