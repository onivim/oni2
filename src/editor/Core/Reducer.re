/*
 * Reducer.re
 *
 * Manage state transitions based on Actions
 */

open Actions;
open Types;

let truncateFilepath = path => {
  switch (path) {
  | Some(p) => Filename.basename(p)
  | None => ""
  };
};

let showTablineTabs = (state: State.t, tabs) => {
  switch (state.configuration.tablineMode) {
  | Tabs =>
    List.map(
      ({tab, name}: Tabline.t) =>
        State.Tab.{id: tab, title: name, active: false},
      tabs,
    )
  | _ => state.tabs
  };
};

let showTablineBuffers = (state: State.t, buffers: list(BufferMetadata.t)) => {
  switch (state.configuration.tablineMode) {
  | Buffers =>
    List.map(
      ({id, filePath, _}: BufferMetadata.t) =>
        State.Tab.{
          id,
          title: filePath |> truncateFilepath,
          active: state.activeBufferId == id,
        },
      buffers,
    )
  | _ => state.tabs
  };
};

let applyBufferUpdate =
    (bufferUpdate: BufferUpdate.t, buffer: option(Buffer.t)) => {
  switch (buffer) {
  | None =>
    print_endline("applyBufferUpdate: NO BUFFER FOUND!");
    None;
  | Some(b) =>
    print_endline(
      "applyBufferUpdate: Buffer update! " ++ string_of_int(bufferUpdate.id),
    );
    Some(Buffer.update(b, bufferUpdate));
  };
};

let reduce: (State.t, Actions.t) => State.t =
  (s, a) => {
    switch (a) {
    | ChangeMode(m) =>
      let ret: State.t = {...s, mode: m};
      ret;
    | CursorMove(b) => {...s, cursorPosition: b}
    | BufferEnter(bs) => {
        ...s,
        activeBufferId: bs.bufferId,
        buffers: BufferMap.updateMetadata(s.buffers, bs.buffers),
        tabs: showTablineBuffers(s, bs.buffers),
      }
    | BufferUpdate(bu) => {
        ...s,
        buffers: BufferMap.update(bu.id, applyBufferUpdate(bu), s.buffers),
      }
    | TablineUpdate(tabs) => {...s, tabs: showTablineTabs(s, tabs)}
    | SetEditorFont(font) => {...s, editorFont: font}
    | SetEditorSize(size) => {...s, size}
    | CommandlineShow(commandline) => {...s, commandline}
    | CommandlineHide(commandline) => {...s, commandline}
    | CommandlineUpdate((position, level)) => {
        ...s,
        commandline: {
          ...s.commandline,
          position,
          level,
        },
      }
    | WildmenuShow(wildmenu) => {...s, wildmenu}
    | WildmenuHide(wildmenu) => {...s, wildmenu}
    | WildmenuSelected(selected) => {
        ...s,
        wildmenu: {
          ...s.wildmenu,
          selected,
        },
      }
    | Noop => s
    };
  };
