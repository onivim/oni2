/*
 * Diagnostics.re
 *
 * This module is responsible for tracking the state of 'diagnostics'
 * (usually errors or warnings) that we render in the buffer view
 * or minimap.
 */

open Oni_Core;
open Oni_Core.Types;

/*
 * The type for diagnostics is a nested map:
 * - First level: URI
 * - Second level: Diagnostic identifier / key.
 *   For example - TypeScript might have keys for both compiler errors and lint warnings
 * - Diagnostic list corresponding to the buffer, key pair
 */
type t = StringMap.t(StringMap.t(list(Diagnostic.t)));

let create = () => StringMap.empty;

let getKeyForUri = (uri: Uri.t) => {
  uri |> Uri.toString;
};

let getKeyForBuffer = (b: Buffer.t) => {
  b |> Buffer.getUri |> Uri.toString;
};

let updateDiagnosticsMap =
    (
      diagnosticsKey,
      diagnostics,
      diagnosticsMap: StringMap.t(list(Diagnostic.t)),
    ) => {
  StringMap.add(diagnosticsKey, diagnostics, diagnosticsMap);
};

let explodeDiagnostics = (buffer, diagnostics) => {
  let f = (prev, curr: Diagnostic.t) => {
    IntMap.update(
      Index.toZeroBasedInt(curr.range.startPosition.line),
      existing =>
        switch (existing) {
        | None => Some([curr])
        | Some(v) => Some([curr, ...v])
        },
      prev,
    );
  };

  List.map(Diagnostic.explode(buffer), diagnostics)
  |> List.flatten
  |> List.fold_left(f, IntMap.empty);
};

let clear = (instance, key) => {
  let f = identifierMap => {
    StringMap.remove(key, identifierMap);
  };

  StringMap.map(f, instance);
};

let change = (instance, uri, diagKey, diagnostics) => {
  let bufferKey = getKeyForUri(uri);

  let updateBufferMap =
      (bufferMap: option(StringMap.t(list(Diagnostic.t)))) => {
    switch (bufferMap) {
    | Some(v) => Some(updateDiagnosticsMap(diagKey, diagnostics, v))
    | None =>
      Some(updateDiagnosticsMap(diagKey, diagnostics, StringMap.empty))
    };
  };

  StringMap.update(bufferKey, updateBufferMap, instance);
};

let getDiagnostics = (instance, buffer) => {
  let f = ((_key, v)) => v;
  let bufferKey = getKeyForBuffer(buffer);
  switch (StringMap.find_opt(bufferKey, instance)) {
  | None => []
  | Some(v) => StringMap.bindings(v) |> List.map(f) |> List.flatten
  };
};

let getDiagnosticsAtPosition = (instance, buffer, position) => {
  getDiagnostics(instance, buffer)
  |> List.filter((d: Diagnostic.t) => Range.contains(d.range, position));
};

let getDiagnosticsMap = (instance, buffer) => {
  getDiagnostics(instance, buffer) |> explodeDiagnostics(buffer);
};
