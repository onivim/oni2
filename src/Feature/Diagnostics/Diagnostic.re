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
  range: CharacterRange.t,
  message: string,
  severity: Exthost.Diagnostic.Severity.t,
};

let create = (~range, ~message, ~severity) => {range, message, severity};

let explode = (buffer, diagnostic) => {
  let lineCount = Buffer.getNumberOfLines(buffer);
  let measure = n => {
    let lineIdx = EditorCoreTypes.LineNumber.toZeroBased(n);
    lineIdx >= 0 && lineIdx < lineCount
      ? buffer
        |> Buffer.getLine(lineIdx)
        // TODO: Is this correct, for a character range?
        |> BufferLine.lengthInBytes
      : 0;
  };

  CharacterRange.explode(measure, diagnostic.range)
  |> ListEx.firstk(Constants.maxDiagnosticLines)
  |> ListEx.safeMap(range =>
       create(
         ~range,
         ~message=diagnostic.message,
         ~severity=diagnostic.severity,
       )
     );
};
