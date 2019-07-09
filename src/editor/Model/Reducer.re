/*
 * Reducer.re
 *
 * Manage state transitions based on Actions
 */

open Actions;

let reduce: (State.t, Actions.t) => State.t =
  (s, a) =>
    if (a == Actions.Tick) {
      s;
    } else {
      let s = {
        ...s,
        buffers: Buffers.reduce(s.buffers, a),
        editorGroups: EditorGroups.reduce(s.editorGroups, a),
        lifecycle: Lifecycle.reduce(s.lifecycle, a),
        syntaxHighlighting:
          SyntaxHighlighting.reduce(s.syntaxHighlighting, a),
        wildmenu: Wildmenu.reduce(s.wildmenu, a),
        commandline: Commandline.reduce(s.commandline, a),
        searchHighlights: SearchHighlights.reduce(a, s.searchHighlights),
        statusBar: StatusBarReducer.reduce(s.statusBar, a),
        fileExplorer: FileExplorer.reduce(s.fileExplorer, a),
      };

      switch (a) {
      | SetLanguageInfo(languageInfo) => {...s, languageInfo}
      | SetIconTheme(iconTheme) => {...s, iconTheme}
      | ChangeMode(m) =>
        let ret: State.t = {...s, mode: m};
        ret;
      | SetEditorFont(font) => {...s, editorFont: font}
      | SetInputControlMode(m) => {...s, inputControlMode: m}
      | CommandlineShow(_) => {...s, inputControlMode: CommandLineFocus}
      | CommandlineHide => {...s, inputControlMode: EditorTextFocus}
      | RegisterDockItem(dock) =>
        switch (dock) {
        | {position: Left, _} => {
            ...s,
            editorLayout: {
              ...s.editorLayout,
              leftDock: [dock, ...s.editorLayout.leftDock],
              dockItems: [dock, ...s.editorLayout.dockItems],
            },
          }
        | {position: Right, _} => {
            ...s,
            editorLayout: {
              ...s.editorLayout,
              rightDock: [dock, ...s.editorLayout.rightDock],
              dockItems: [dock, ...s.editorLayout.dockItems],
            },
          }
        }
      | RemoveDockItem(id) => {
          ...s,
          editorLayout: WindowManager.removeDockItem(~id, s.editorLayout),
        }
      | AddDockItem(id) =>
        switch (WindowManager.findDockItem(id, s.editorLayout)) {
        | Some(dock) => {
            ...s,
            editorLayout: {
              ...s.editorLayout,
              leftDock: s.editorLayout.leftDock @ [dock],
            },
          }
        | None => s
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
