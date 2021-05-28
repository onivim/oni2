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
  isUnused: bool,
  isDeprecated: bool,
  tags: list(Exthost.Diagnostic.Tag.t),
};

let create = (~range, ~message, ~severity, ~tags) => {
  let isUnused =
    tags |> List.exists(tag => tag == Exthost.Diagnostic.Tag.Unused);
  let isDeprecated =
    tags |> List.exists(tag => tag == Exthost.Diagnostic.Tag.Deprecated);
  {range, message, severity, tags, isUnused, isDeprecated};
};

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
         ~tags=diagnostic.tags,
       )
     );
};
