/*
 * BufferWrap.rei
 */

open Oni_Core.Types;

type t;

let create: (Buffer.t, int) => t;

let getVirtualLine: (int, Buffer.t, t) => string;
let getVirtualLineCount: t => int;

let bufferPositionToVirtual: (Position.t, t) => Position.t;

let bufferRangeToVirtual: (Range.t, t) => list(Range.t);

let update: (BufferUpdate.t, t) => t;
