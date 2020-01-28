/*
 * Buffer.rei
 *
 * In-memory text buffer representation
 */

open CamomileBundled.Camomile;

exception OutOfBounds;

type t;

let empty: t;

let make: (~indentation: IndentationSettings.t, string) => t;

let lengthInBytes: t => int;

let raw: t => string;

/*
 * [lengthSlow(bufferLine)] returns the UTF-8 length of the buffer line.
 *
 * SLOW: Note that this requires traversal of the entire string.
 * Using [lengthBounded] is recommended as an alternative, when possible.
 */
let lengthSlow: t => int;

/*
 * [lengthBounded(~max, str)] returns the minimum of the UTF-8 length of the buffer line, OR [max].
 *
 * This is faster than [lengthSlow] because it does not require traversing the entire string.
 */
let lengthBounded: (~max: int, t) => int;

/*
  * [getUCharExn(~index, str)] returns the [UChar.t] at UTF-8 index [index].
  * Raises [OutOfBounds] if the index is not valid.
 */
let getUCharExn: (~index: int, t) => UChar.t;

let subExn: (~index: int, ~length: int, t) => string;

let getPositionAndWidth: (~index: int, t) => (int, int);
