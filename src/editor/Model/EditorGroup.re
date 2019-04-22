/*
 * EditorGroup.re
 *
 * Manage a group of editors
*/

open Oni_Core;

type t = {
    activeEditorId: int,
    editors: IntMap.t(Editor.t),
};

let create = () => {
    let defaultEditor = Editor.create();
    let editors = IntMap.empty |> IntMap.add(defaultEditor.id, defaultEditor);
    { editors, activeEditorId: defaultEditor.id };
};

let getEditorById = (id: int, v: t) => {
    IntMap.find(id, v.editors);
};

let getActiveEditor = (v: t) => {
    getEditorById(v.activeEditorId, v);
}

let reduce = (v: t, action: Actions.t) => v;
