/*
 * BufferLine.rei
 *
 * In-memory text buffer representation
 */

open EditorCoreTypes;

exception OutOfBounds;

type t;

type measure = Uchar.t => float;

let make: (~measure: measure, string) => t;

let empty: (~measure: measure, unit) => t;

let lengthInBytes: t => int;

let raw: t => string;

let measure: (t, Uchar.t) => float;

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
let lengthBounded: (~max: CharacterIndex.t, t) => int;

/*
 * [getIndex(~byte, str)] returns the character index at byte [byte]
 */
let getIndex: (~byte: ByteIndex.t, t) => CharacterIndex.t;

/*
 * [getByteFromIndex(~index, str)] returns the byte index at character [index].
 */
let getByteFromIndex: (~index: CharacterIndex.t, t) => ByteIndex.t;

/*
  * [getUcharExn(~index, str)] returns the [Uchar.t] at UTF-8 index [index].
  * Raises [OutOfBounds] if the index is not valid.
 */
let getUcharExn: (~index: CharacterIndex.t, t) => Uchar.t;

let subExn: (~index: CharacterIndex.t, ~length: int, t) => string;

let getPixelPositionAndWidth: (~index: CharacterIndex.t, t) => (float, float);

let getLeadingWhitespacePixels: t => float;

let traverse:
  (
    ~maxDistance: int=?,
    ~f: Uchar.t => bool,
    ~direction: [ | `Backwards | `Forwards],
    ~index: CharacterIndex.t,
    t
  ) =>
  CharacterIndex.t;

module Slow: {
  /*
   * [getIndexFromPixel(~position, str)] returns the character index at pixel position [position].
   * The position of a character is dependent on indentation settings, multi-width characters, etc.
   *
   * _slow_ because requires traversal of the string, currently.
   */
  let getIndexFromPixel: (~pixel: float, t) => CharacterIndex.t;

  let getByteFromPixel:
    (~relativeToByte: ByteIndex.t=?, ~pixelX: float, t) => ByteIndex.t;
};
