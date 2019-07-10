/*
 * Selectors.re
 *
 * Helpers to map the State.t to more usable values
 */

open Oni_Core;

let getActiveEditorGroup = (state: State.t) => {
  EditorGroups.getActiveEditorGroup(state.editorGroups);
};

let getEditorGroupById = (state: State.t, id) => {
  EditorGroups.getEditorGroupById(state.editorGroups, id);
};

let getActiveEditor = (editorGroup: option(EditorGroup.t)) => {
  switch (editorGroup) {
  | None => None
  | Some(v) => EditorGroup.getActiveEditor(v)
  };
};

let getBufferById = (state: State.t, id: int) => {
  Buffers.getBuffer(id, state.buffers);
};

let getBufferForEditor = (state: State.t, editor: Editor.t) => {
  Buffers.getBuffer(editor.bufferId, state.buffers);
};

let getMatchingPairs = (state: State.t, bufferId: int) => {
  switch (state.configuration.default.editorMatchBrackets) {
  | false => None
  | true => {
    switch (IntMap.find_opt(bufferId, state.searchHighlights)) {
    | Some(v) => v.matchingPair
    | None => None
  }
  }
  }
};

let getSearchHighlights = (state: State.t, bufferId: int) => {
  switch (IntMap.find_opt(bufferId, state.searchHighlights)) {
  | Some(v) => v.highlightRanges
  | None => IntMap.empty
  };
};

let getActiveBuffer = (state: State.t) => {
  let editorOpt = state |> getActiveEditorGroup |> getActiveEditor;

  switch (editorOpt) {
  | Some(editor) => getBufferForEditor(state, editor)
  | None => None
  };
};
