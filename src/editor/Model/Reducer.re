/*
 * Reducer.re
 *
 * Manage state transitions based on Actions
 */

open Actions;
open Oni_Core.Types;

let sortTabsById = tabs =>
  State.Tab.(List.sort((t1, t2) => compare(t1.id, t2.id), tabs));

let truncateFilepath = path =>
  switch (path) {
  | Some(p) => Filename.basename(p)
  | None => "untitled"
  };

let showTablineTabs = (state: State.t, tabs) =>
  switch (state.configuration.editorTablineMode) {
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
  switch (state.configuration.editorTablineMode) {
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
  (s, a) =>
    if (a == Actions.Tick) {
      s;
    } else {
      let s = {
        ...s,
        editor:
          Editor.reduce(
            s.editor,
            a,
            BufferMap.getBuffer(s.activeBufferId, s.buffers),
          ),
        lifecycle: Lifecycle.reduce(s.lifecycle, a),
        syntaxHighlighting:
          SyntaxHighlighting.reduce(s.syntaxHighlighting, a),
        wildmenu: Wildmenu.reduce(s.wildmenu, a),
        commandline: Commandline.reduce(s.commandline, a),
        statusBar: StatusBarReducer.reduce(s.statusBar, a),
      };

      switch (a) {
      | SetLanguageInfo(languageInfo) => {...s, languageInfo}
      | SetIconTheme(iconTheme) => {...s, iconTheme}
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
      | BufferUpdate(bu) => {
          ...s,
          buffers: BufferMap.update(bu.id, applyBufferUpdate(bu), s.buffers),
        }
      | TablineUpdate(tabs) => {...s, tabs: showTablineTabs(s, tabs)}
      | SetEditorFont(font) => {...s, editorFont: font}
      | TextChanged({activeBufferId, modified})
      | TextChangedI({activeBufferId, modified}) => {
          ...s,
          tabs: updateTabs(activeBufferId, modified, s.tabs),
        }
      | SetInputControlMode(m) => {...s, inputControlMode: m}
      | CommandlineShow(_) => {...s, inputControlMode: NeovimMenuFocus}
      | CommandlineHide(_) => {...s, inputControlMode: EditorTextFocus}
      | AddLeftDock(dock) => {
          ...s,
          editorLayout: {
            ...s.editorLayout,
            leftDock: [dock, ...s.editorLayout.leftDock],
          },
        }
      | AddRightDock(dock) => {
          ...s,
          editorLayout: {
            ...s.editorLayout,
            rightDock: [dock, ...s.editorLayout.rightDock],
          },
        }
      | AddSplit(split) =>
        let id = WindowManager.WindowId.current();
        {
          ...s,
          editorLayout: {
            ...s.editorLayout,
            activeWindowId: id,
            windows:
              WindowManager.addSplit(id, split, s.editorLayout.windows),
          },
        };
      | RemoveSplit(id) => {
          ...s,
          editorLayout: {
            ...s.editorLayout,
            windows: WindowManager.removeSplit(id, s.editorLayout.windows),
          },
        }
      | _ => s
      };
    };
