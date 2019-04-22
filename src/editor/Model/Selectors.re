/*
 * Selectors.re
 *
 * Helpers to map the State.t to more usable values
 */

open Oni_Core;

let getActiveEditor = (state: State.t) => {
  EditorGroup.getActiveEditor(state.editors);
};

let getBufferById = (state: State.t, id: int) => {
  BufferMap.getBuffer(id, state.buffers);
};

let getBufferForEditor = (state: State.t, editor: Editor.t) => {
  BufferMap.getBuffer(editor.bufferId, state.buffers);
};

let getActiveBuffer = (state: State.t) => {
  state |> getActiveEditor |> getBufferForEditor(state);
};
