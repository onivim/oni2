/*
 * Diagnostic.re
 */

open EditorCoreTypes;
open Oni_Core;

[@deriving show]
type t = {
  range: Range.t,
  message: string,
};

let create = (~range, ~message, ()) => {range, message};

let explode = (buffer, diagnostic) => {
  let measure = n => Buffer.getLineLength(buffer, Index.toOneBased(n));

  Range.explode(measure, diagnostic.range)
  |> List.map(range => create(~range, ~message=diagnostic.message, ()));
};
