/*
 * Diagnostic.re
 */

open EditorCoreTypes;
open Oni_Core;
open Oni_Core.Utility;

[@deriving show]
type t = {
  range: Range.t,
  message: string,
};

let create = (~range, ~message, ()) => {range, message};

// Clamp the number of lines a diagnostic range can span to a tractable number.
let maxDiagnosticLines = 1000;

let explode = (buffer, diagnostic) => {
  let lineCount = Buffer.getNumberOfLines(buffer);
  let measure = n => {
    Index.toZeroBased(n) < lineCount
      ? buffer
        |> Buffer.getLine(Index.toZeroBased(n))
        |> BufferLine.lengthInBytes
      : 0;
  };

  Range.explode(measure, diagnostic.range)
  |> ListEx.firstk(maxDiagnosticLines)
  |> ListEx.safeMap(range => create(~range, ~message=diagnostic.message, ()));
};
