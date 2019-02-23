/*
 * Reducer.re
 *
 * Manage state transitions based on Actions
 */

open Actions;
open Types;

let sortTabsById = tabs =>
  State.Tab.(List.sort((t1, t2) => compare(t1.id, t2.id), tabs));

let truncateFilepath = path =>
  switch (path) {
  | Some(p) => Filename.basename(p)
  | None => ""
  };

let showTablineTabs = (state: State.t, tabs) =>
  switch (state.configuration.tablineMode) {
  | Tabs =>
    List.map(
      ({tab, name}: Tabline.t) =>
        State.Tab.{id: tab, title: name, active: false, modified: false},
      tabs,
    )
    |> sortTabsById
  | _ => state.tabs
  };

let showTablineBuffers = (state: State.t, buffers, activeBufferId) =>
  switch (state.configuration.tablineMode) {
  | Buffers =>
    List.map(
      ({id, filePath, modified, _}: BufferMetadata.t) =>
        State.Tab.{
          id,
          title: filePath |> truncateFilepath,
          active: activeBufferId == id,
          modified,
        },
      buffers,
    )
    |> sortTabsById
  | _ => state.tabs
  };

let updateTabs = (bufId, modified, tabs) =>
  State.Tab.(
    List.map(tab => tab.id === bufId ? {...tab, modified} : tab, tabs)
    |> sortTabsById
  );

let applyBufferUpdate = (bufferUpdate, buffer) =>
  switch (buffer) {
  | None => None
  | Some(b) => Some(Buffer.update(b, bufferUpdate))
  };

let reduce: (State.t, Actions.t) => State.t =
  (s, a) => {
    let s = {
      ...s,
      editorView:
        EditorView.reduce(
          s.editorView,
          a,
          BufferMap.getBuffer(s.activeBufferId, s.buffers),
          s.editorFont,
        ),
    };

    switch (a) {
    | ChangeMode(m) =>
      let ret: State.t = {...s, mode: m};
      ret;
    | BufferWritePost(bs) => {
        ...s,
        activeBufferId: bs.bufferId,
        buffers: BufferMap.updateMetadata(s.buffers, bs.buffers),
        tabs: showTablineBuffers(s, bs.buffers, bs.bufferId),
      }
    | BufferDelete(bd) => {
        ...s,
        tabs: showTablineBuffers(s, bd.buffers, bd.bufferId),
      }
    | BufferEnter(bs) => {
        ...s,
        activeBufferId: bs.bufferId,
        buffers: BufferMap.updateMetadata(s.buffers, bs.buffers),
        tabs: showTablineBuffers(s, bs.buffers, bs.bufferId),
      }
    | BufferUpdate(bu) =>
      let r = {
        ...s,
        buffers: BufferMap.update(bu.id, applyBufferUpdate(bu), s.buffers),
      };
      BufferMap.log(r.buffers);
      r;
    | TablineUpdate(tabs) => {...s, tabs: showTablineTabs(s, tabs)}
    | SetEditorFont(font) => {...s, editorFont: font}
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
    | TextChanged({activeBufferId, modified})
    | TextChangedI({activeBufferId, modified}) => {
        ...s,
        tabs: updateTabs(activeBufferId, modified, s.tabs),
      }
    | _ => s
    };
  };
