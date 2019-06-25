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
  let activeId = defaultGroup.id;
  let idToGroup = IntMap.add(activeId, defaultGroup, IntMap.empty);
  {idToGroup, activeId, lastEditorFont: None};
};

let getEditorGroupById = (v: t, id) => {
  IntMap.find_opt(id, v.idToGroup);
};

let getActiveEditorGroup = (v: t) => {
  getEditorGroupById(v, v.activeId);
};

let reduce = (v: t, action: Actions.t) => {
  switch (action) {
  | SetEditorFont(ef) => {...v, lastEditorFont: Some(ef)}
  | EditorGroupAdd(editorGroup) =>
    let editorGroup =
      switch (v.lastEditorFont) {
      | Some(ef) => EditorGroupReducer.reduce(editorGroup, SetEditorFont(ef))
      | None => editorGroup
      };

    {
      ...v,
      activeId: editorGroup.id,
      idToGroup: IntMap.add(editorGroup.id, editorGroup, v.idToGroup),
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
    }
  };
};
