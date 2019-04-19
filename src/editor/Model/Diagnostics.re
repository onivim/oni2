/*
 * Diagnostics.re
 *
 * This module is responsible for tracking the state of 'diagnostics'
 * (usually errors or warnings) that we render in the buffer view
 * or minimap.
 */

open Oni_Core;
open Oni_Core.Types;

module Diagnostic = {
  type t = {range: Range.t};

  let create = (~range: Range.t, ()) => {
    let ret: t = {range: range};
    ret;
  };
};

/*
 * The type for diagnostics is a nested map:
 * - First level: Buffer Path
 * - Second level: Diagnostic identifier / key.
 *   For example - TypeScript might have keys for both compiler errors and lint warnings
 * - Diagnostic list corresponding to the buffer, key pair
 */
type t = StringMap.t(StringMap.t(list(Diagnostic.t)));

let create = () => StringMap.empty;

let _getKeyForBuffer = (b: Buffer.t) => {
  b |> Buffer.getUri |> Uri.toString;
};

let _updateDiagnosticsMap =
    (
      diagnosticsKey,
      diagnostics,
      diagnosticsMap: StringMap.t(list(Diagnostic.t)),
    ) => {
  StringMap.add(diagnosticsKey, diagnostics, diagnosticsMap);
};

let change = (instance, buffer, diagKey, diagnostics) => {
  let bufferKey = _getKeyForBuffer(buffer);

  let updateBufferMap =
      (bufferMap: option(StringMap.t(list(Diagnostic.t)))) => {
    switch (bufferMap) {
    | Some(v) => Some(_updateDiagnosticsMap(diagKey, diagnostics, v))
    | None =>
      Some(_updateDiagnosticsMap(diagKey, diagnostics, StringMap.empty))
    };
  };

  StringMap.update(bufferKey, updateBufferMap, instance);
};

let getDiagnostics = (_instance, _buffer) => {

    [
        Diagnostic.create(~range=Range.zero, ()),
    ]

    /* []; */

  /* let f = ((_key, v)) => v; */

  /* let bufferKey = _getKeyForBuffer(buffer); */
  /* switch (StringMap.find_opt(bufferKey, instance)) { */
  /* | None => [] */
  /* | Some(v) => StringMap.bindings(v) |> List.map(f) |> List.flatten */
  /* }; */
};
