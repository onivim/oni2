/*
 * Buffer.rei
 *
 * In-memory text buffer representation
 */

open CamomileBundled.Camomile;

type t;

let make: (~indentation: IndentationSettings.t, string) => t;

let lengthInBytes: t => int;

let raw: t => string;

let slowLengthUtf8: t => int;
let boundedLengthUtf8: (~max: int, t) => int;

/*let char(~byteIndex, t) => option(Char.t);
  let unsafeChar(~byteIndex, t) => Char.t;*/

let unsafeGetUChar: (~index: int, t) => UChar.t;
let unsafeSub: (~index: int, ~length: int, t) => string;

let getPositionAndWidth: (~index: int, t) => (int, int);
