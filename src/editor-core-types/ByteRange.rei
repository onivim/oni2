[@deriving show]
type t = {
  start: BytePosition.t,
  stop: BytePosition.t,
};

let zero: t;

let contains: (BytePosition.t, t) => bool;

/**
 * [toHash(ranges)] takes a list [ranges] of [Range.t], and returns them as a
 * a hash table, where the key is the start line of the [Range.t],
 * and the value is the list of all ranges with that start line.
 */
let toHash: list(t) => Hashtbl.t(LineNumber.t, list(t));

let equals: (t, t) => bool;

/**
 * [compare(a, b)] returns:
 * - 0 if a & b are equal
 * - a negative integer if a is less than b
 * - a positive integer if a is greater than b
 */
let compare: (t, t) => int;

/**
 * [normalize(range)] returns a new range, with the [start] less than
 * or equal to the [stop]
 */
let normalize: t => t;
