/*
 * EditorGroup.re
 *
 * Manage a group of editors
*/

open Oni_Core;

type t = {
    activeEditorId: int,
    editors: IntMap.t(Editor.t),
    bufferIdToEditorId: IntMap.t(int),
};

let create = () => {
    let defaultEditor = Editor.create();
    let editors = IntMap.empty |> IntMap.add(defaultEditor.id, defaultEditor);
    { editors, bufferIdToEditorId: IntMap.empty, activeEditorId: defaultEditor.id };
};

let getEditorById = (id: int, v: t) => {
    IntMap.find(id, v.editors);
};

let getActiveEditor = (v: t) => {
    getEditorById(v.activeEditorId, v);
};

let getOrCreateEditorForBuffer = (state: t, bufferId: int) => {

    switch (IntMap.find_opt(bufferId, state.bufferIdToEditorId)) {
    | Some(v) => (state, v)
    | None => {
        let newEditor = Editor.create(~bufferId, ());
        let newState = {
            ...state,
            editors: IntMap.add(newEditor.id, newEditor, state.editors),
            bufferIdToEditorId: IntMap.add(bufferId, newEditor.id, state.bufferIdToEditorId),
        };
        (newState, newEditor.id)
    }
    }
    
};

let reduce = (v: t, action: Actions.t, ) => {

    let editors = IntMap.fold((key, value, prev) => {
        IntMap.add(key, Editor.reduce(value, action), prev);
    }, v.editors, IntMap.empty);

    let v = {
        ...v,
        editors,
    };

    switch (action) {
    | BufferEnter(bs) => {
        let (newState, activeEditorId) = getOrCreateEditorForBuffer(v, bs.bufferId);
        {
            ...newState,
            activeEditorId,
        }
        
    }
    | _ => v
    }
    
};
