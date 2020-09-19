/*
 * Diagnostics.re
 *
 * This module is responsible for tracking the state of 'diagnostics'
 * (usually errors or warnings) that we render in the buffer view
 * or minimap.
 */

open EditorCoreTypes;
open Oni_Core;
open Oni_Core.Utility;

module Diagnostic = Diagnostic;

// MODEL

module DiagnosticEntry = {
  type t = {
    uri: Uri.t,
    owner: string,
    diagnostics: list(Diagnostic.t),
  };

  let ofExthost =
      (owner: string, entries: list(Exthost.Msg.Diagnostics.entry)) => {
    let exthostDiagToDiag: Exthost.Diagnostic.t => Diagnostic.t =
      extDiag => {
        let range = Exthost.OneBasedRange.toRange(extDiag.range);
        let message = extDiag.message;
        Diagnostic.create(~range, ~message, ());
      };

    let exthostEntryToEntry: Exthost.Msg.Diagnostics.entry => t =
      entry => {
        let diagnostics = List.map(exthostDiagToDiag, snd(entry));
        let uri = fst(entry);
        {owner, uri, diagnostics};
      };

    entries |> List.map(exthostEntryToEntry);
  };
};

[@deriving show]
type msg =
  | Set([@opaque] list(DiagnosticEntry.t))
  | Clear({owner: string});

module Msg = {
  let exthost =
    fun
    | Exthost.Msg.Diagnostics.Clear({owner}) => Clear({owner: owner})
    | Exthost.Msg.Diagnostics.ChangeMany({owner, entries}) =>
      Set(DiagnosticEntry.ofExthost(owner, entries));

  let diagnostics = (uri, owner, diagnostics) => {
    Set([DiagnosticEntry.{uri, owner, diagnostics}]);
  };

  let clear = (~owner) => Clear({owner: owner});
};

/*
 * The type for diagnostics is a nested map:
 * - First level: URI
 * - Second level: Diagnostic identifier / key.
 *   For example - TypeScript might have keys for both compiler errors and lint warnings
 * - Diagnostic list corresponding to the buffer, key pair
 */
type model = {
  keyToUri: StringMap.t(Uri.t),
  diagnosticsMap: StringMap.t(StringMap.t(list(Diagnostic.t))),
  // Keep a cached count so we don't have to recalculate when
  // querying UI
  count: int,
};

let initial = {
  keyToUri: StringMap.empty,
  diagnosticsMap: StringMap.empty,
  count: 0,
};

// UPDATE

let count = (diags: model) => diags.count;

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
      EditorCoreTypes.LineNumber.toZeroBased(curr.range.start.line),
      existing =>
        switch (existing) {
        | None => Some([curr])
        | Some(v) => Some([curr, ...v])
        },
      prev,
    );
  };

  ListEx.safeMap(Diagnostic.explode(buffer), diagnostics)
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

let update = (msg, model) =>
  switch (msg) {
  | Clear({owner}) => clear(model, owner)
  | Set(entries) =>
    entries
    |> List.fold_left(
         (acc, curr: DiagnosticEntry.t) => {
           change(acc, curr.uri, curr.owner, curr.diagnostics)
         },
         model,
       )
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

let getAllDiagnostics = (diagnostics: model) => {
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
  |> List.filter((Diagnostic.{range, _}) =>
       CharacterRange.contains(position, range)
     );
};

let getDiagnosticsMap = (instance, buffer) => {
  getDiagnostics(instance, buffer) |> _explodeDiagnostics(buffer);
};

module View = {
  open Revery.UI;
  open Oni_Components;

  module Colors = Feature_Theme.Colors;

  module Styles = {
    open Style;

    let pane = [flexGrow(1), flexDirection(`Row)];

    let noResultsContainer = [
      flexGrow(1),
      alignItems(`Center),
      justifyContent(`Center),
    ];

    let title = (~theme) => [
      color(Colors.PanelTitle.activeForeground.from(theme)),
      margin(8),
    ];
  };

  let toLocListItem = (diagWithUri: (Uri.t, Diagnostic.t)) => {
    let (uri, diag) = diagWithUri;
    let file = Uri.toFileSystemPath(uri);
    let location = Diagnostic.(diag.range.start);
    LocationList.{file, location, text: diag.message, highlight: None};
  };
  let make =
      (
        ~onSelectFile:
           (
             ~filePath: string,
             ~position: EditorCoreTypes.CharacterPosition.t
           ) =>
           unit,
        ~diagnostics,
        ~theme,
        ~uiFont: UiFont.t,
        ~editorFont,
        (),
      ) => {
    let items =
      diagnostics
      |> getAllDiagnostics
      |> List.map(toLocListItem)
      |> Array.of_list;

    let onSelectItem = (item: LocationList.item) => {
      onSelectFile(~filePath=item.file, ~position=item.location);
    };

    let innerElement =
      if (Array.length(items) == 0) {
        <View style=Styles.noResultsContainer>
          <Text
            style={Styles.title(~theme)}
            fontFamily={uiFont.family}
            fontSize={uiFont.size}
            text="No problems, yet!"
          />
        </View>;
      } else {
        <LocationList theme uiFont editorFont items onSelectItem />;
      };

    <View style=Styles.pane> innerElement </View>;
  };
};
