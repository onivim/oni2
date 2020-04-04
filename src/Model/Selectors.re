/*
 * Selectors.re
 *
 * Helpers to map the State.t to more usable values
 */

open Oni_Core;
open Oni_Core.Utility;

module Ext = Oni_Extensions;
module Editor = Feature_Editor.Editor;

let getActiveEditorGroup = (state: State.t) => {
  EditorGroups.getActiveEditorGroup(state.editorGroups);
};

let getEditorGroupById = (state: State.t, id) => {
  EditorGroups.getEditorGroupById(state.editorGroups, id);
};

let getActiveEditor = (editorGroup: option(EditorGroup.t)) => {
  switch (editorGroup) {
  | None => None
  | Some(v) => EditorGroup.getActiveEditor(v)
  };
};

let getBufferById = (state: State.t, id: int) => {
  Buffers.getBuffer(id, state.buffers);
};

let getBufferForEditor = (state: State.t, editor: Editor.t) => {
  Buffers.getBuffer(editor.bufferId, state.buffers);
};

let getConfigurationValue = (state: State.t, buffer: Buffer.t, f) => {
  let fileType =
    Ext.LanguageInfo.getLanguageFromBuffer(state.languageInfo, buffer);
  Configuration.getValue(~fileType, f, state.configuration);
};

let getActiveBuffer = (state: State.t) => {
  let editorOpt = state |> getActiveEditorGroup |> getActiveEditor;

  switch (editorOpt) {
  | Some(editor) => getBufferForEditor(state, editor)
  | None => None
  };
};

let withActiveBufferAndFileType = (state: State.t, f) => {
  let () =
    getActiveBuffer(state)
    |> OptionEx.flatMap(buf =>
         Buffer.getFileType(buf) |> Option.map(ft => (buf, ft))
       )
    |> Option.iter(((buf, ft)) => f(buf, ft));
  ();
};

let getActiveConfigurationValue = (state: State.t, f) => {
  switch (getActiveBuffer(state)) {
  | None => Configuration.getValue(f, state.configuration)
  | Some(buffer) =>
    let fileType =
      Ext.LanguageInfo.getLanguageFromBuffer(state.languageInfo, buffer);
    Configuration.getValue(~fileType, f, state.configuration);
  };
};

let getActiveTerminal = (state: State.t) => {
  state
  // See if terminal has focus
  |> getActiveBuffer
  |> Option.map(Oni_Core.Buffer.getId)
  |> Option.map(id => BufferRenderers.getById(id, state.bufferRenderers))
  |> OptionEx.flatMap(renderer =>
       switch (renderer) {
       | BufferRenderer.Terminal(terminal) => Some(terminal)
       | _ => None
       }
     );
};

let getActiveTerminalId = (state: State.t) => {
  state
  |> getActiveTerminal
  |> Option.map(({id, _}: BufferRenderer.terminal) => id);
};

let terminalIsActive = (state: State.t) =>
  getActiveTerminalId(state) != None;
