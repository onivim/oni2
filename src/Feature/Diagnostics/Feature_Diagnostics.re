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
        Diagnostic.create(
          ~range,
          ~message,
          ~severity=extDiag.severity,
          ~tags=extDiag.tags,
        );
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
type command =
  | ToggleProblemsPane;

[@deriving show]
type msg =
  | Set([@opaque] list(DiagnosticEntry.t))
  | Clear({owner: string})
  | Pane(Pane.msg)
  | Command(command);

type outmsg =
  | Nothing
  | OpenFile({
      filePath: string,
      position: EditorCoreTypes.CharacterPosition.t,
    })
  | PreviewFile({
      filePath: string,
      position: EditorCoreTypes.CharacterPosition.t,
    })
  | TogglePane({paneId: string});

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
  pane: Pane.model,
};

let initial = {
  keyToUri: StringMap.empty,
  diagnosticsMap: StringMap.empty,
  count: 0,
  pane: Pane.initial,
};

// UPDATE

let count = (diags: model) => diags.count;

let getKeyForUri = (uri: Uri.t) => {
  uri |> Uri.toString;
};

let getKeyForBuffer = (b: Buffer.t) => {
  b |> Buffer.getUri |> Uri.toString;
};

module Internal = {
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

  let recalculateCount = diagnostics => {
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

  let mapRanges = (f, buffer, model) => {
    let bufferKey = getKeyForBuffer(buffer);

    let diagnosticsMap' =
      model.diagnosticsMap
      |> StringMap.update(
           bufferKey,
           Option.map(diagKeyToRanges => {
             diagKeyToRanges
             |> StringMap.map(diagnostics => {
                  diagnostics
                  |> List.map((diagnostic: Diagnostic.t) => {
                       let range' = f(diagnostic.range);
                       {...diagnostic, range: range'};
                     })
                })
           }),
         );

    {...model, diagnosticsMap: diagnosticsMap'};
  };
  let filterRanges = (f, buffer, model) => {
    let bufferKey = getKeyForBuffer(buffer);

    let diagnosticsMap' =
      model.diagnosticsMap
      |> StringMap.update(
           bufferKey,
           Option.map(diagKeyToRanges => {
             diagKeyToRanges
             |> StringMap.map(diagnostics => {
                  diagnostics
                  |> List.filter((diagnostic: Diagnostic.t) => {
                       f(diagnostic.range)
                     })
                })
           }),
         );

    {...model, diagnosticsMap: diagnosticsMap'};
  };
};

let clear = (instance, key) => {
  let f = identifierMap => {
    StringMap.remove(key, identifierMap);
  };

  {...instance, diagnosticsMap: StringMap.map(f, instance.diagnosticsMap)}
  |> Internal.recalculateCount;
};

let change = (instance, uri, diagKey, diagnostics) => {
  let bufferKey = getKeyForUri(uri);

  let keyToUri = StringMap.add(bufferKey, uri, instance.keyToUri);

  let updateBufferMap =
      (bufferMap: option(StringMap.t(list(Diagnostic.t)))) => {
    switch (bufferMap) {
    | Some(v) => Some(Internal.updateDiagnosticsMap(diagKey, diagnostics, v))
    | None =>
      Some(
        Internal.updateDiagnosticsMap(diagKey, diagnostics, StringMap.empty),
      )
    };
  };

  {
    ...instance,
    keyToUri,
    diagnosticsMap:
      StringMap.update(bufferKey, updateBufferMap, instance.diagnosticsMap),
  }
  |> Internal.recalculateCount;
};

let diagnosticToLocList = (diagWithUri: (Uri.t, Diagnostic.t)) => {
  let (uri, diag) = diagWithUri;
  let file = Uri.toFileSystemPath(uri);
  let location = Diagnostic.(diag.range.start);
  Oni_Components.LocationListItem.{
    file,
    location,
    text: diag.message,
    highlight: None,
  };
};

let getAllDiagnostics = (diagnostics: model) => {
  let extractBindings = map => {
    StringMap.bindings(map) |> List.map(snd) |> List.flatten;
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

let syncPane = (model: model) => {
  let locations = model |> getAllDiagnostics |> List.map(diagnosticToLocList);
  {...model, pane: Pane.setDiagnostics(locations, model.pane)};
};

let update = (~previewEnabled, msg, model) => {
  switch (msg) {
  | Command(ToggleProblemsPane) => (
      model,
      TogglePane({paneId: "workbench.panel.markers"}),
    )
  | Pane(msg) =>
    let (pane', outmsg) = Pane.update(msg, model.pane);
    let eff =
      switch (outmsg) {
      | Component_VimTree.Nothing => Nothing
      | Component_VimTree.Touched(item) =>
        previewEnabled
          ? PreviewFile({filePath: item.file, position: item.location})
          : OpenFile({filePath: item.file, position: item.location})
      | Component_VimTree.SelectedNode(_) => Nothing
      | Component_VimTree.Selected(item) =>
        OpenFile({filePath: item.file, position: item.location})
      | Component_VimTree.Collapsed(_) => Nothing
      | Component_VimTree.Expanded(_) => Nothing
      };
    ({...model, pane: pane'}, eff);
  | Clear({owner}) => (clear(model, owner) |> syncPane, Nothing)
  | Set(entries) =>
    let model' =
      entries
      |> List.fold_left(
           (acc, curr: DiagnosticEntry.t) => {
             change(acc, curr.uri, curr.owner, curr.diagnostics)
           },
           model,
         )
      |> syncPane;
    (model', Nothing);
  };
};

let getDiagnostics = ({diagnosticsMap, _}, buffer) => {
  let f = ((_key, v)) => v;
  let bufferKey = getKeyForBuffer(buffer);
  switch (StringMap.find_opt(bufferKey, diagnosticsMap)) {
  | None => []
  | Some(v) => StringMap.bindings(v) |> List.map(f) |> List.flatten
  };
};

let getDiagnosticsAtPosition = (instance, buffer, position) => {
  getDiagnostics(instance, buffer)
  |> List.filter((Diagnostic.{range, _}) =>
       CharacterRange.contains(position, range)
     );
};
let getDiagnosticsInRange = (model, buffer, query) => {
  getDiagnostics(model, buffer)
  |> List.filter((Diagnostic.{range, _}) =>
       CharacterRange.intersects(query, range)
     );
};

let getDiagnosticsMap = (instance, buffer) => {
  getDiagnostics(instance, buffer) |> Internal.explodeDiagnostics(buffer);
};

let maxSeverity = diagnostics => {
  open Exthost.Diagnostic.Severity;
  let rec loop = (currentSeverity, remaining: list(Diagnostic.t)) =>
    if (currentSeverity == Error) {
      Error;
    } else {
      switch (remaining) {
      | [] => currentSeverity
      | [hd, ...tail] => loop(max(currentSeverity, hd.severity), tail)
      };
    };

  loop(Hint, diagnostics);
};

let moveMarkers = (~newBuffer, ~markerUpdate, model: model) => {
  let shiftLines = (~afterLine, ~delta, model: model) => {
    model
    |> Internal.mapRanges(
         CharacterRange.shiftLine(~afterLine, ~delta),
         newBuffer,
       );
  };

  let clearLine = (~line, model) => {
    model
    |> Internal.filterRanges(
         (range: CharacterRange.t) =>
           !
             EditorCoreTypes.LineNumber.(
               range.start.line == line && range.stop.line == line
             ),
         newBuffer,
       );
  };

  let shiftCharacters =
      (
        ~line,
        ~afterByte as _,
        ~deltaBytes as _,
        ~afterCharacter,
        ~deltaCharacters,
        model,
      ) =>
    model
    |> Internal.mapRanges(
         CharacterRange.shiftCharacters(
           ~line,
           ~afterCharacter,
           ~delta=deltaCharacters,
         ),
         newBuffer,
       );

  MarkerUpdate.apply(
    ~clearLine,
    ~shiftLines,
    ~shiftCharacters,
    markerUpdate,
    model,
  );
};

module Commands = {
  open Feature_Commands.Schema;
  let problems =
    define(
      ~category="View",
      ~title="Toggle Problems (Errors, Warnings)",
      "workbench.actions.view.problems",
      Command(ToggleProblemsPane),
    );
};

module Keybindings = {
  open Feature_Input.Schema;
  let toggleProblems =
    bind(
      ~key="<S-C-M>",
      ~command=Commands.problems.id,
      ~condition=WhenExpr.Value(True),
    );

  let toggleProblemsOSX =
    bind(
      ~key="<D-S-M>",
      ~command=Commands.problems.id,
      ~condition="isMac" |> WhenExpr.parse,
    );
};

module Contributions = {
  let commands = Commands.[problems];

  let keybindings = Keybindings.[toggleProblems, toggleProblemsOSX];

  let pane =
    Pane.pane
    |> Feature_Pane.Schema.map(
         ~msg=msg => Pane(msg),
         ~model=({pane, _}) => pane,
       );
};

module Testing = {
  let change = change;
  let clear = clear;
};
