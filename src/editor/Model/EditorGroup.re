/*
  * EditorGroup.re
  *
  * Manage a group of editors
 */

open Oni_Core;

module EditorGroupId =
  Revery.UniqueId.Make({});

type t = {
  id: int,
  activeEditorId: option(int),
  editors: IntMap.t(Editor.t),
  bufferIdToEditorId: IntMap.t(int),
  reverseTabOrder: list(int),
  metrics: EditorMetrics.t,
};

let create = () => {
  {
    id: EditorGroupId.getUniqueId(),
    editors: IntMap.empty,
    bufferIdToEditorId: IntMap.empty,
    activeEditorId: None,
    reverseTabOrder: [],
    metrics: EditorMetrics.create(),
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

let rec getIndexOfElement = (l, elem) => {
  switch (l) {
  | [] => (-1)
  | [hd, ...tl] => hd === elem ? 0 : getIndexOfElement(tl, elem) + 1
  };
};

let _getAdjacentEditor = (editor: int, reverseTabOrder: list(int)) => {
  switch (getIndexOfElement(reverseTabOrder, editor)) {
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

let isActiveEditor = (state, editorId) => {
	switch (state.activeEditorId) {
	| None => false
	| Some(v) => v == editorId;
	}
}

let removeEditorById = (state, editorId) => {
  switch (IntMap.find_opt(editorId, state.editors)) {
  | None => state
  | Some(v) =>
    let bufferId = v.bufferId;
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

    let ret: t = {
      ...state,
      activeEditorId: newActiveEditorId,
      editors,
      bufferIdToEditorId,
      reverseTabOrder: filteredTabList,
    };
    ret;
  };
};

let removeEditorsForBuffer = (state, bufferId) => {
  switch (IntMap.find_opt(bufferId, state.bufferIdToEditorId)) {
  | None => state
  | Some(v) => removeEditorById(state, v)
  };
};

let reduce = (v: t, action: Actions.t) => {
  let metrics = EditorMetrics.reduce(v.metrics, action);

  /* Only send updates to _active_ editor */
  let editors =
    switch (v.activeEditorId, getActiveEditor(v)) {
    | (Some(id), Some(e)) =>
      IntMap.add(id, Editor.reduce(e, action, metrics), v.editors)
    | _ => v.editors
    };

  let v = {...v, metrics, editors};

  switch (action) {
  | BufferEnter({id, _}) =>
	prerr_endline ("BUFFER ENTER: " ++ show(v));
    let (newState, activeEditorId) = getOrCreateEditorForBuffer(v, id);
    {...newState, activeEditorId: Some(activeEditorId)};
  | ViewCloseEditor(id) => removeEditorById(v, id)
  | _ => v
  };
};
