/*
 * Diagnostics.re
 *
 * This module is responsible for tracking the state of 'diagnostics'
 * (usually errors or warnings) that we render in the buffer view
 * or minimap.
 */

open EditorCoreTypes;
open Oni_Core;
open Oni_Core_Kernel;

/*
 * The type for diagnostics is a nested map:
 * - First level: URI
 * - Second level: Diagnostic identifier / key.
 *   For example - TypeScript might have keys for both compiler errors and lint warnings
 * - Diagnostic list corresponding to the buffer, key pair
 */
type t = {
  keyToUri: StringMap.t(Uri.t),
  diagnosticsMap: StringMap.t(StringMap.t(list(Diagnostic.t))),
  // Keep a cached count so we don't have to recalculate when
  // querying UI
  count: int,
};

let create = () => {
  keyToUri: StringMap.empty,
  diagnosticsMap: StringMap.empty,
  count: 0,
};

let count = (diags: t) => diags.count;

let getKeyForUri = (uri: Uri.t) => {
  uri |> Uri.toString;
};

let getKeyForBuffer = (b: Buffer.t) => {
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

let _explodeDiagnostics = (buffer, diagnostics) => {
  let f = (prev, curr: Diagnostic.t) => {
    IntMap.update(
      Index.toZeroBased(curr.range.start.line),
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

let _recalculateCount = diagnostics => {
  let foldKeys = keyMap => {
    StringMap.fold(
      (_key, curr, acc) => {acc + List.length(curr)},
      keyMap,
      0,
    );
  };

  let count =
    StringMap.fold(
      (_key, keyMap, acc) => {acc + foldKeys(keyMap)},
      diagnostics.diagnosticsMap,
      0,
    );

  {...diagnostics, count};
};

let clear = (instance, key) => {
  let f = identifierMap => {
    StringMap.remove(key, identifierMap);
  };

  {...instance, diagnosticsMap: StringMap.map(f, instance.diagnosticsMap)}
  |> _recalculateCount;
};

let change = (instance, uri, diagKey, diagnostics) => {
  let bufferKey = getKeyForUri(uri);

  let keyToUri = StringMap.add(bufferKey, uri, instance.keyToUri);

  let updateBufferMap =
      (bufferMap: option(StringMap.t(list(Diagnostic.t)))) => {
    switch (bufferMap) {
    | Some(v) => Some(_updateDiagnosticsMap(diagKey, diagnostics, v))
    | None =>
      Some(_updateDiagnosticsMap(diagKey, diagnostics, StringMap.empty))
    };
  };

  {
    ...instance,
    keyToUri,
    diagnosticsMap:
      StringMap.update(bufferKey, updateBufferMap, instance.diagnosticsMap),
  }
  |> _recalculateCount;
};

let getDiagnostics = ({diagnosticsMap, _}, buffer) => {
  let f = ((_key, v)) => v;
  let bufferKey = getKeyForBuffer(buffer);
  switch (StringMap.find_opt(bufferKey, diagnosticsMap)) {
  | None => []
  | Some(v) => StringMap.bindings(v) |> List.map(f) |> List.flatten
  };
};

let _value = ((_key, v)) => v;

let getAllDiagnostics = (diagnostics: t) => {
  let extractBindings = map => {
    StringMap.bindings(map) |> List.map(_value) |> List.flatten;
  };

  StringMap.fold(
    (k, v, accum) => {
      let uri = StringMap.find_opt(k, diagnostics.keyToUri);
      switch (uri) {
      | None => accum
      | Some(uri) =>
        let diags = extractBindings(v) |> List.map(v => (uri, v));
        [diags, ...accum];
      };
    },
    diagnostics.diagnosticsMap,
    [],
  )
  |> List.flatten;
};

let getDiagnosticsAtPosition = (instance, buffer, position) => {
  getDiagnostics(instance, buffer)
  |> List.filter((Diagnostic.{range, _}) => Range.contains(position, range));
};

let getDiagnosticsMap = (instance, buffer) => {
  getDiagnostics(instance, buffer) |> _explodeDiagnostics(buffer);
};
