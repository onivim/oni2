/*
 * BufferWrap.rei
 */

open Oni_Core;

type t;

let create: (Buffer.t, int) => t;

let getVirtualLine: (int, Buffer.t, t) => string;

let bufferRangeToVirtualPosition: (Position.t, t) => Position.t;

let bufferRangeToVirtualRanges: (Range.t, t) => list(Range.t);

let update: (BufferUpdate.t, t) => t;
