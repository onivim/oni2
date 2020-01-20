/*
 * Diagnostic.rei
 */

open EditorCoreTypes;
open Oni_Core;
open Oni_Core_Kernel;

type t = {
  range: Range.t,
  message: string,
};

let create: (~range: Range.t, ~message: string, unit) => t;

/*
  [explode(buffer, diagnostic)] splits up a multi-line diagnostic into a diagnostic per-line
 */
let explode: (Buffer.t, t) => list(t);

let pp: (Format.formatter, t) => unit;
