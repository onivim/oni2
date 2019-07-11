/*
 * EditorGroups.re
 *
 * Managing an aggregate of EditorGroups
 */

open Oni_Core;
open Oni_Core.Types;

type t = {
  idToGroup: IntMap.t(EditorGroup.t),
  activeId: int,
  /* Cache the last editor font, so when a new group is created, we can share it */
  lastEditorFont: option(EditorFont.t),
};

let create = () => {
  let defaultGroup = EditorGroup.create();
  let activeId = defaultGroup.editorGroupId;
  let idToGroup = IntMap.add(activeId, defaultGroup, IntMap.empty);
  {idToGroup, activeId, lastEditorFont: None};
};

let getEditorGroupById = (v: t, id) => {
  IntMap.find_opt(id, v.idToGroup);
};

let getActiveEditorGroup = (v: t) => {
  getEditorGroupById(v, v.activeId);
};

let applyToAllEditorGroups =
    (editors: IntMap.t(EditorGroup.t), action: Actions.t) => {
  IntMap.map(eg => EditorGroupReducer.reduce(eg, action), editors);
};

/* Validate 'activeId' is set to a valid editor group,
   otherwise move to the first valid */
let ensureActiveId = (v: t) => {
  switch (IntMap.find_opt(v.activeId, v.idToGroup)) {
  | Some(_) => v
  | None =>
    switch (IntMap.min_binding_opt(v.idToGroup)) {
    | Some((key, _)) => {...v, activeId: key}
    | _ => v
    }
  };
};

let removeEmptyEditorGroups = (v: t) => {
  let idToGroup =
    IntMap.filter((key, v) => !EditorGroup.isEmpty(v), v.idToGroup);

  {...v, idToGroup};
};

let reduce = (v: t, action: Actions.t) => {
  switch (action) {
  | SetEditorFont(ef) => {
      ...v,
      idToGroup: applyToAllEditorGroups(v.idToGroup, action),
      lastEditorFont: Some(ef),
    }
  | WindowSetActive(_, editorGroupId) => {...v, activeId: editorGroupId}
  | EditorGroupAdd(editorGroup) =>
    let editorGroup =
      switch (v.lastEditorFont) {
      | Some(ef) => EditorGroupReducer.reduce(editorGroup, SetEditorFont(ef))
      | None => editorGroup
      };

    {
      ...v,
      activeId: editorGroup.editorGroupId,
      idToGroup:
        IntMap.add(editorGroup.editorGroupId, editorGroup, v.idToGroup),
    };
  | EditorGroupSetSize(editorGroupId, _) =>
    let idToGroup =
      IntMap.update(
        editorGroupId,
        editorGroup =>
          switch (editorGroup) {
          | Some(eg) => Some(EditorGroupReducer.reduce(eg, action))
          | None => None
          },
        v.idToGroup,
      );

    {...v, idToGroup};
  | action =>
    let ret =
      switch (getActiveEditorGroup(v)) {
      | Some(eg) => {
          ...v,
          idToGroup:
            IntMap.add(
              v.activeId,
              EditorGroupReducer.reduce(eg, action),
              v.idToGroup,
            ),
        }
      | None => v
      };

    switch (action) {
    | ViewCloseEditor(_) => ret |> removeEmptyEditorGroups |> ensureActiveId
    | _ => ret
    };
  };
};
