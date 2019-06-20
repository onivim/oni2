/*
  * EditorGroup.re
  *
  * Manage a group of editors
 */

open Oni_Core;

let reduce = (v: EditorGroup.t, action: Actions.t) => {
  let metrics = EditorMetricsReducer.reduce(v.metrics, action);

  /* Only send updates to _active_ editor */
  let editors =
    switch (v.activeEditorId, EditorGroup.getActiveEditor(v)) {
    | (Some(id), Some(e)) =>
      IntMap.add(id, Editor.reduce(e, action, metrics), v.editors)
    | _ => v.editors
    };

  let v = {...v, metrics, editors};

  switch (action) {
  | BufferEnter({id, _}) =>
    let (newState, activeEditorId) =
      EditorGroup.getOrCreateEditorForBuffer(v, id);
    {...newState, activeEditorId: Some(activeEditorId)};
  | ViewCloseEditor(id) => EditorGroup.removeEditorById(v, id)
  | ViewSetActiveEditor(id) =>
    switch (IntMap.find_opt(id, v.editors)) {
    | None => v
    | Some(_) => {...v, activeEditorId: Some(id)}
    }
  | _ => v
  };
};
