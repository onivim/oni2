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
            windowManager: {
              ...s.windowManager,
              leftDock: [dock, ...s.windowManager.leftDock],
              dockItems: [dock, ...s.windowManager.dockItems],
            },
          }
        | {position: Right, _} => {
            ...s,
            windowManager: {
              ...s.windowManager,
              rightDock: [dock, ...s.windowManager.rightDock],
              dockItems: [dock, ...s.windowManager.dockItems],
            },
          }
        }
      | RemoveDockItem(id) => {
          ...s,
          windowManager: WindowManager.removeDockItem(~id, s.windowManager),
        }
      | AddDockItem(id) =>
        switch (WindowManager.findDockItem(id, s.windowManager)) {
        | Some(dock) => {
            ...s,
            windowManager: {
              ...s.windowManager,
              leftDock: s.windowManager.leftDock @ [dock],
            },
          }
        | None => s
        }
      | WindowSetActive(splitId, _) => {
        ...s,
        windowManager: {
          ...s.windowManager,
          activeWindowId: splitId
        }
      }
      | WindowTreeSetSize(width, height) => {
          ...s,
          windowManager:
            WindowManager.setTreeSize(width, height, s.windowManager),
        }
      | AddSplit(direction, split) => {
          ...s,
          windowManager: {
            ...s.windowManager,
            activeWindowId: split.id,
            windowTree:
              WindowTree.addSplit(
                ~target=Some(s.windowManager.activeWindowId),
                direction,
                split,
                s.windowManager.windowTree,
              ),
          },
        }
      | RemoveSplit(id) => {
          ...s,
          windowManager: {
            ...s.windowManager,
            windowTree:
              WindowTree.removeSplit(id, s.windowManager.windowTree),
          },
        }
      | _ => s
      };
    };
