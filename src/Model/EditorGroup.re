/*
  * EditorGroup.re
  *
  * Manage a group of editors
 */

open Oni_Core;
open Feature_Editor;

module EditorGroupId =
  Revery.UniqueId.Make({});

[@deriving show]
type t = {
  editorGroupId: int,
  activeEditorId: option(int),
  editors: [@opaque] IntMap.t(Editor.t),
  bufferIdToEditorId: [@opaque] IntMap.t(int),
  reverseTabOrder: list(int),
  metrics: EditorMetrics.t,
};

let create: unit => t =
  () => {
    editorGroupId: EditorGroupId.getUniqueId(),
    editors: IntMap.empty,
    bufferIdToEditorId: IntMap.empty,
    activeEditorId: None,
    reverseTabOrder: [],
    metrics: EditorMetrics.create(),
  };

let getEditorById = (id, model) => IntMap.find_opt(id, model.editors);

let getMetrics = model => model.metrics;

let getActiveEditor = model =>
  switch (model.activeEditorId) {
  | Some(id) => getEditorById(id, model)
  | None => None
  };

let setActiveEditor = (model, editorId) => {
  ...model,
  activeEditorId: Some(editorId),
};

let getOrCreateEditorForBuffer = (state, bufferId) => {
  switch (IntMap.find_opt(bufferId, state.bufferIdToEditorId)) {
  | Some(editor) => (state, editor)
  | None =>
    let newEditor = Editor.create(~bufferId, ());
    let newState = {
      ...state,
      editors: IntMap.add(newEditor.editorId, newEditor, state.editors),
      bufferIdToEditorId:
        IntMap.add(bufferId, newEditor.editorId, state.bufferIdToEditorId),
      reverseTabOrder: [newEditor.editorId, ...state.reverseTabOrder],
    };
    (newState, newEditor.editorId);
  };
};

// TODO: Just use List.find_opt?
let rec _getIndexOfElement = elem =>
  fun
  | [] => (-1)
  | [hd, ...tl] => hd === elem ? 0 : _getIndexOfElement(elem, tl) + 1;

let _getAdjacentEditor = (editor: int, reverseTabOrder: list(int)) => {
  switch (_getIndexOfElement(editor, reverseTabOrder)) {
  | (-1) => None
  | idx =>
    switch (
      List.nth_opt(reverseTabOrder, idx + 1),
      List.nth_opt(reverseTabOrder, max(idx - 1, 0)),
    ) {
    | (Some(next), _) => Some(next)
    | (_, Some(prev)) => Some(prev)
    | _ => None
    }
  };
};

let isEmpty = model => IntMap.is_empty(model.editors);

let removeEditorById = (state, editorId) => {
  switch (IntMap.find_opt(editorId, state.editors)) {
  | None => state
  | Some(editor) =>
    let bufferId = editor.bufferId;
    let filteredTabList =
      List.filter(t => editorId != t, state.reverseTabOrder);
    let bufferIdToEditorId =
      IntMap.remove(bufferId, state.bufferIdToEditorId);
    let editors = IntMap.remove(editorId, state.editors);

    let newActiveEditorId =
      switch (state.activeEditorId) {
      | None => None
      | Some(currentEditorId) =>
        if (currentEditorId === editorId) {
          _getAdjacentEditor(currentEditorId, state.reverseTabOrder);
        } else {
          /* We're not removing the current editor, so we can just leave it */
          Some(
            currentEditorId,
          );
        }
      };

    {
      ...state,
      activeEditorId: newActiveEditorId,
      editors,
      bufferIdToEditorId,
      reverseTabOrder: filteredTabList,
    };
  };
};
