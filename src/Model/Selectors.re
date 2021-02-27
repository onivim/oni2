/*
 * Selectors.re
 *
 * Helpers to map the State.t to more usable values
 */

open Oni_Core;
open Oni_Core.Utility;

module Editor = Feature_Editor.Editor;

let getBufferById = (state: State.t, id: int) => {
  Feature_Buffers.get(id, state.buffers);
};

let getBufferForEditor = (buffers, editor: Editor.t) => {
  Feature_Buffers.get(Editor.getBufferId(editor), buffers);
};

let getConfigurationValue = (state: State.t, buffer: Buffer.t, f) => {
  let fileType =
    Option.value(
      ~default=Exthost.LanguageInfo.defaultLanguage,
      Buffer.getFileType(buffer) |> Buffer.FileType.toOption,
    );
  Feature_Configuration.Legacy.getValue(~fileType, f, state.config);
};

let getActiveBuffer = (state: State.t) => {
  state.layout
  |> Feature_Layout.activeEditor
  |> getBufferForEditor(state.buffers);
};

let withActiveBufferAndFileType = (state: State.t, f) => {
  let () =
    getActiveBuffer(state)
    |> Option.map(buf => {
         let fileType = Buffer.getFileType(buf) |> Buffer.FileType.toString;
         (buf, fileType);
       })
    |> Option.iter(((buf, ft)) => f(buf, ft));
  ();
};

let getActiveConfigurationValue = (state: State.t, f) => {
  switch (getActiveBuffer(state)) {
  | None => Feature_Configuration.Legacy.getValue(f, state.config)
  | Some(buffer) =>
    let fileType = Buffer.getFileType(buffer) |> Buffer.FileType.toString;

    Feature_Configuration.Legacy.getValue(~fileType, f, state.config);
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

let configResolver = (state: State.t) => {
  let maybeActiveBuffer = getActiveBuffer(state);
  let fileType =
    maybeActiveBuffer
    |> Option.map(Buffer.getFileType)
    |> Option.map(Buffer.FileType.toString)
    |> Option.value(~default=Buffer.FileType.default);

  Feature_Configuration.resolver(~fileType, state.config, state.vim);
};

let mode = (state: State.t) => {
  state.layout |> Feature_Layout.activeEditor |> Feature_Editor.Editor.mode;
};
