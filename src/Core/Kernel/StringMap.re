/*
 * StringMap.re
 *
 * Map from string -> 'a
 */

include Map.Make({
  type t = string;
  let compare = String.compare;
});
