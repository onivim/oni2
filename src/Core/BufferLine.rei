/*
 * Buffer.rei
 *
 * In-memory text buffer representation
 */

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
 * [getIndex(~byte, str)] returns the character index at byte [byte]
 */
let getIndex: (~byte: int, t) => int;

/*
 * [getByteFromIndex(~index, str)] returns the byte index at character [index].
 */
let getByteFromIndex: (~index: int, t) => int;

/*
  * [getUcharExn(~index, str)] returns the [Uchar.t] at UTF-8 index [index].
  * Raises [OutOfBounds] if the index is not valid.
 */
let getUcharExn: (~index: int, t) => Uchar.t;

let subExn: (~index: int, ~length: int, t) => string;

let getPositionAndWidth: (~index: int, t) => (int, int);

module Slow: {
  /*
   * [getByteFromPosition(~position, str)] returns the byte index as position [index].
   * The position of a character is dependent on indentation settings, multi-width characters, etc.
   *
   * _slow_ because requires traversal of the string, currently.
   */
  let getByteFromPosition: (~startByte: int=?, ~position: int, t) => int;
};
