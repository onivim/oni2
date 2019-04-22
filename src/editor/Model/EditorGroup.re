/*
  * EditorGroup.re
  *
  * Manage a group of editors
 */

open Oni_Core;

type t = {
  activeEditorId: option(int),
  editors: IntMap.t(Editor.t),
  bufferIdToEditorId: IntMap.t(int),
  reverseTabOrder: list(int),
};

let create = () => {
  {
    /* let defaultEditor = Editor.create(); */
    /* let editors = IntMap.empty |> IntMap.add(defaultEditor.id, defaultEditor); */

    editors: IntMap.empty,
    bufferIdToEditorId: IntMap.empty,
    activeEditorId: None,
    reverseTabOrder: [],
  };
};

let getEditorById = (id: int, v: t) => {
  IntMap.find(id, v.editors);
};

let getActiveEditor = (v: t) => {
  switch (v.activeEditorId) {
  | Some(id) => Some(getEditorById(id, v))
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
  | BufferEnter(bufferId) =>
    let (newState, activeEditorId) = getOrCreateEditorForBuffer(v, bufferId);
    {...newState, activeEditorId: Some(activeEditorId)};
  | _ => v
  };
};
