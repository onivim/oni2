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

let hasEditor = (~editorId, model) => {
  IntMap.mem(editorId, model.editors);
};

let setActiveEditor = (model, editorId) => {
  switch (IntMap.find_opt(editorId, model.editors)) {
  | None => model
  | Some(_) => {...model, activeEditorId: Some(editorId)}
  };
};

let setBufferFont = (~bufferId, ~font, group) => {
  let editors =
    group.editors
    |> IntMap.map((editor: Feature_Editor.Editor.t) =>
         if (Editor.getBufferId(editor) == bufferId) {
           Editor.setFont(~font, editor);
         } else {
           editor;
         }
       );

  {...group, editors};
};

let count = ({editors, _}) => IntMap.bindings(editors) |> List.length;

let getOrCreateEditorForBuffer = (~font, ~buffer, state) => {
  let bufferId = Feature_Editor.EditorBuffer.id(buffer);
  switch (IntMap.find_opt(bufferId, state.bufferIdToEditorId)) {
  | Some(editor) => (state, editor)
  | None =>
    let newEditor = Editor.create(~font, ~buffer, ());
    let newState = {
      ...state,
      editors: IntMap.add(Editor.getId(newEditor), newEditor, state.editors),
      bufferIdToEditorId:
        IntMap.add(
          bufferId,
          Editor.getId(newEditor),
          state.bufferIdToEditorId,
        ),
      reverseTabOrder: [Editor.getId(newEditor), ...state.reverseTabOrder],
    };
    (newState, Editor.getId(newEditor));
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

let isEmpty = model => IntMap.is_empty(model.editors);

let removeEditorById = (state, editorId) => {
  switch (IntMap.find_opt(editorId, state.editors)) {
  | None => state
  | Some(editor) =>
    let bufferId = Feature_Editor.Editor.getBufferId(editor);
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

let removeEditorsForBuffer = (~bufferId, group) => {
  IntMap.fold(
    (editorId, editor, acc) => {
      let editorBufferId = Editor.getBufferId(editor);

      if (editorBufferId == bufferId) {
        removeEditorById(acc, editorId);
      } else {
        acc;
      };
    },
    group.editors,
    group,
  );
};

let updateEditor = (~editorId, msg, group) => {
  group.editors
  |> IntMap.find_opt(editorId)
  |> Option.map(editor => {
       let (editor', outmsg) = Feature_Editor.update(editor, msg);
       let editors' = group.editors |> IntMap.add(editorId, editor');

       ({...group, editors: editors'}, Some(outmsg));
     })
  |> Option.value(~default=(group, None));
};
