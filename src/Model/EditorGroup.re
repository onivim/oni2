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
};

let create: unit => t =
  () => {
    editorGroupId: EditorGroupId.getUniqueId(),
    editors: IntMap.empty,
    bufferIdToEditorId: IntMap.empty,
    activeEditorId: None,
    reverseTabOrder: [],
  };

let getEditorById = (id, model) => IntMap.find_opt(id, model.editors);

let getActiveEditor = model =>
  switch (model.activeEditorId) {
  | Some(id) => getEditorById(id, model)
  | None => None
  };

let setActiveEditor = (model, editorId) => {
  ...model,
  activeEditorId: Some(editorId),
};

let setBufferFont = (~bufferId, ~font, group) => {
  let editors =
    group.editors
    |> IntMap.map((editor: Feature_Editor.Editor.t) =>
         if (editor.bufferId == bufferId) {
           Editor.setFont(~font, editor);
         } else {
           editor;
         }
       );

  {...group, editors};
};

let getOrCreateEditorForBuffer = (~font, ~bufferId, state) => {
  switch (IntMap.find_opt(bufferId, state.bufferIdToEditorId)) {
  | Some(editor) => (state, editor)
  | None =>
    let newEditor = Editor.create(~font, ~bufferId, ());
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

let rec _getIndexOfElement = elem =>
  fun
  | [] => None
  | [hd, ...tl] =>
    hd === elem
      ? Some(0)
      : (
        switch (_getIndexOfElement(elem, tl)) {
        | None => None
        | Some(i) => Some(i + 1)
        }
      );

let _getAdjacentEditor = (editor: int, reverseTabOrder: list(int)) => {
  switch (_getIndexOfElement(editor, reverseTabOrder)) {
  | None => None
  | Some(idx) =>
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

let setActiveEditorTo = (kind, model) =>
  switch (model.reverseTabOrder) {
  | []
  | [_] => model
  | _ =>
    switch (model.activeEditorId) {
    | Some(activeEditorId) =>
      switch (_getIndexOfElement(activeEditorId, model.reverseTabOrder)) {
      | None => model
      | Some(idx) =>
        // The diff amounts are inverted because the list is in reverse order
        let newIndex =
          switch (kind) {
          | `Next => idx - 1
          | `Previous => idx + 1
          };

        let count = List.length(model.reverseTabOrder);

        let newIndex =
          if (newIndex < 0) {
            // Wrapping negative, go to end
            count - 1;
          } else if (newIndex >= count) {
            0;
            // If this is past the end, go to zero
          } else {
            newIndex;
          };

        {
          ...model,
          activeEditorId: List.nth_opt(model.reverseTabOrder, newIndex),
        };
      }
    | None => model
    }
  };

let nextEditor = setActiveEditorTo(`Next);
let previousEditor = setActiveEditorTo(`Previous);

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
