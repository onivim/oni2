[@deriving show]
type t = {
  start: CharacterPosition.t,
  stop: CharacterPosition.t,
};
/*
 * explode(range, measure) takes a Range.t and a measurement function (int => int),
 * and expands a multiple-line range into a list of ranges, where there is
 * a single range per-line.
 *
 * If the input range is a single line, a single item list with the input range
 * will be returned.
 */
let explode: (LineNumber.t => int, t) => list(t);

let contains: (CharacterPosition.t, t) => bool;
