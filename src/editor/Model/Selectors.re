/*
 * Selectors.re
 *
 * Helpers to map the State.t to more usable values
 */

let getActiveEditor = (state: State.t) => {
    IntMap.find(state.activeEditor, state.editors)
};

let getActiveBuffer = (state: State.t) => {
    let editor = getActiveEditor(state);
    IntMap.find_opt(editor.bufferId, state.buffers);
}
