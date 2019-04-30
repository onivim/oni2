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
  [@deriving show({with_path: false})]
  type t = {range: Range.t};

  let create = (~range: Range.t, ()) => {
    let ret: t = {range: range};
    ret;
  };

  let explode = (buffer: Buffer.t, v: t) => {
    let measure = Buffer.getLineLength(buffer);

    Range.explode(measure, v.range) |> List.map(range => create(~range, ()));
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

let change = (instance, buffer, diagKey, diagnostics) => {
  let bufferKey = getKeyForBuffer(buffer);

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
  | None => IntMap.empty
  | Some(v) =>
    StringMap.bindings(v)
    |> List.map(f)
    |> List.flatten
    |> explodeDiagnostics(buffer)
  };
};
