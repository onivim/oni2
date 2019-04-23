/*
  * EditorGroup.re
  *
  * Manage a group of editors
 */

/* TEMPORARY */
Printexc.record_backtrace(true);

open Oni_Core;

type t = {
  activeEditorId: option(int),
  editors: IntMap.t(Editor.t),
  bufferIdToEditorId: IntMap.t(int),
  reverseTabOrder: list(int),
};

let create = () => {
  {
    editors: IntMap.empty,
    bufferIdToEditorId: IntMap.empty,
    activeEditorId: None,
    reverseTabOrder: [],
  };
};

let show = (v: t) => {
  IntMap.fold(
    (key, v: Editor.t, prev) =>
      prev
      ++ " |Editor: "
      ++ string_of_int(key)
      ++ " (buffer: "
      ++ string_of_int(v.bufferId)
      ++ ")|",
    v.editors,
    "",
  );
};

let getEditorById = (id: int, v: t) => {
  IntMap.find_opt(id, v.editors);
};

let getActiveEditor = (v: t) => {
  switch (v.activeEditorId) {
  | Some(id) =>
    switch (getEditorById(id, v)) {
    | Some(v) => Some(v)
    | None => None
    }
  | None => None
  };
};

let getOrCreateEditorForBuffer = (state: t, bufferId: int) => {
  switch (IntMap.find_opt(bufferId, state.bufferIdToEditorId)) {
  | Some(v) => (state, v)
  | None =>
    let newEditor = Editor.create(~bufferId, ());
    let newState = {
      ...state,
      editors: IntMap.add(newEditor.id, newEditor, state.editors),
      bufferIdToEditorId:
        IntMap.add(bufferId, newEditor.id, state.bufferIdToEditorId),
      reverseTabOrder: [newEditor.id, ...state.reverseTabOrder],
    };
    (newState, newEditor.id);
  };
};

let _getAdjacentEditor = (editor: int, reverseTabOrder: list(int)) => {
  switch (List.find_opt(v => v === editor, reverseTabOrder)) {
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

let removeEditorsForBuffer = (state, bufferId) => {
  switch (IntMap.find_opt(bufferId, state.bufferIdToEditorId)) {
  | None => state
  | Some(v) =>
    let filteredTabList = List.filter(t => v != t, state.reverseTabOrder);
    let bufferIdToEditorId =
      IntMap.remove(bufferId, state.bufferIdToEditorId);
    let editors = IntMap.remove(v, state.editors);

    let newActiveEditorId =
      switch (state.activeEditorId) {
      | None => None
      | Some(currentEditorId) =>
        if (currentEditorId === v) {
          _getAdjacentEditor(currentEditorId, state.reverseTabOrder);
        } else {
          /* We're not removing the current editor, so we can just leave it */
          Some(
            currentEditorId,
          );
        }
      };

    let ret: t = {
      activeEditorId: newActiveEditorId,
      editors,
      bufferIdToEditorId,
      reverseTabOrder: filteredTabList,
    };
    ret;
  };
};

let reduce = (v: t, action: Actions.t) => {
  let editors =
    IntMap.fold(
      (key, value, prev) =>
        IntMap.add(key, Editor.reduce(value, action), prev),
      v.editors,
      IntMap.empty,
    );

  let v = {...v, editors};

  switch (action) {
  | BufferEnter({id, _}) =>
    let (newState, activeEditorId) = getOrCreateEditorForBuffer(v, id);
    {...newState, activeEditorId: Some(activeEditorId)};
  | _ => v
  };
};
