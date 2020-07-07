/*
 * Selectors.re
 *
 * Helpers to map the State.t to more usable values
 */

open Oni_Core;
open Oni_Core.Utility;

module Editor = Feature_Editor.Editor;

let getBufferById = (state: State.t, id: int) => {
  Buffers.getBuffer(id, state.buffers);
};

let getBufferForEditor = (buffers, editor: Editor.t) => {
  Buffers.getBuffer(Editor.getBufferId(editor), buffers);
};

let getConfigurationValue = (state: State.t, buffer: Buffer.t, f) => {
  let fileType =
    Exthost.LanguageInfo.getLanguageFromBuffer(state.languageInfo, buffer);
  Configuration.getValue(~fileType, f, state.configuration);
};

let getActiveBuffer = (state: State.t) => {
  state.layout
  |> Feature_Layout.activeEditor
  |> getBufferForEditor(state.buffers);
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
      Exthost.LanguageInfo.getLanguageFromBuffer(state.languageInfo, buffer);
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

let getBufferForTerminal = (~terminalId: int, state: State.t) => {
  state.bufferRenderers.rendererById
  |> IntMap.filter((_bufferId, renderer) => {
       switch (renderer) {
       | BufferRenderer.Terminal({id, _}) => id == terminalId
       | _ => false
       }
     })
  |> IntMap.choose_opt
  |> Option.map(fst)
  |> OptionEx.flatMap(Vim.Buffer.getById);
};

let getActiveTerminalId = (state: State.t) => {
  state |> getActiveTerminal |> Option.map((Feature_Terminal.{id, _}) => id);
};

let terminalIsActive = (state: State.t) =>
  getActiveTerminalId(state) != None;
