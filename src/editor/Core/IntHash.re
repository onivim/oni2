/*
 * IntHash.re
 *
 * Hashtbl from int -> 'a
 */

include Hashtbl.Make({
  type t = int;
  let equal = (i, j) => i == j;
  let hash = (i) => i land max_int;
});
