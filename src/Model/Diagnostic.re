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
  let lineCount = Buffer.getNumberOfLines(buffer);
  let measure = n =>
    Index.toZeroBased(n) < lineCount
      ? Buffer.getLineLength(buffer, Index.toZeroBased(n)) : 0;

  Range.explode(measure, diagnostic.range)
  |> List.map(range => create(~range, ~message=diagnostic.message, ()));
};
