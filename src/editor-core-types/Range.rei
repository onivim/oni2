[@deriving show]
type t = {
  start: Location.t,
  stop: Location.t,
};

let create: (~start: Location.t, ~stop: Location.t) => t;

/*
 * explode(range, measure) takes a Range.t and a measurement function (int => int),
 * and expands a multiple-line range into a list of ranges, where there is
 * a single range per-line.
 *
 * If the input range is a single line, a single item list with the input range
 * will be returned.
 */
let explode: (Index.t => int, t) => list(t);

/**
 * [toHash(ranges)] takes a list [ranges] of [Range.t], and returns them as a
 * a hash table, where the key is the start line of the [Range.t],
 * and the value is the list of all ranges with that start line.
 */
let toHash: list(t) => Hashtbl.t(Index.t, list(t));

let equals: (t, t) => bool;

let contains: (Location.t, t) => bool;

let toString: t => string;

let minLine: list(t) => option(LineNumber.t);
let maxLine: list(t) => option(LineNumber.t);
