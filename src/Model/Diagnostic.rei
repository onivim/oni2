/*
 * Diagnostic.rei
 */

open Oni_Core;

type t = {
  range: Range.t,
  message: string,
};

let create: (~range: Range.t, ~message: string, unit) => t;

/* 
 [explode(buffer, diagnostic)] splits up a multi-line diagnostic into a diagnostic per-line
*/
let explode: (Buffer.t, t) => list(t);
