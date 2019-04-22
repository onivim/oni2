/*
 * EditorGroup.re
 *
 * Manage a group of editors
*/

type t = {
    activeEditorId: int,
    editors: IntMap.t(Editor.t)
};

let create = () => {
    let defaultEditor = Editor.create();
    let editors = IntMap.empty |> IntMap.add(defaultEditor.id, defaultEditor);
};

let getEditorById = (id: int, v: t) => {
    IntMap.find(id, v);
}
