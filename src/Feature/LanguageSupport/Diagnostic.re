/*
 * Diagnostic.re
 */

open EditorCoreTypes;
open Oni_Core;
open Oni_Core.Utility;

module Constants = {
  // Clamp the number of lines a diagnostic range can span to a tractable number.
  // See: https://github.com/onivim/oni2/issues/1607
  let maxDiagnosticLines = 1000;
};

[@deriving show]
type t = {
  range: Range.t,
  message: string,
};

let create = (~range, ~message, ()) => {range, message};

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
