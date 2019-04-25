/*
 * EditorGroups.re
 *
 * Managing an aggregate of EditorGroups
 */

open Oni_Core;

type t = {
  idToGroup: IntMap.t(EditorGroup.t),
  activeId: int,
};

let create = () => {
  let defaultGroup = EditorGroup.create();
  let activeId = defaultGroup.id;
  let idToGroup = IntMap.add(activeId, defaultGroup, IntMap.empty);
  {idToGroup, activeId};
};

let getEditorGroupById = (v: t, id) => {
  IntMap.find_opt(id, v.idToGroup);
};

let getActiveEditorGroup = (v: t) => {
  getEditorGroupById(v, v.activeId);
};

let reduce = (v: t, action: Actions.t) => {
  let idToGroup =
    IntMap.fold(
      (key, value, prev) =>
        IntMap.add(key, EditorGroupReducer.reduce(value, action), prev),
      v.idToGroup,
      IntMap.empty,
    );

  let v = {...v, idToGroup};

  switch (action) {
  | EditorGroupAdd(editor) => {
      ...v,
      activeId: editor.id,
      idToGroup: IntMap.add(editor.id, editor, v.idToGroup),
    }
  | _ => v
  };
};
