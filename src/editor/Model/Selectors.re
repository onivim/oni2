/*
 * Selectors.re
 *
 * Helpers to map the State.t to more usable values
 */

open Oni_Core;

let getActiveEditor = (state: State.t) => {
    IntMap.find(state.activeEditorId, state.editors)
};

let getActiveBuffer = (state: State.t) => {
    let editor = getActiveEditor(state);
    BufferMap.getBuffer(editor.bufferId, state.buffers);
}
