/*
 * Reducer.re
 *
 * Manage state transitions based on Actions
 */

open Actions;

let reduce: (State.t, Actions.t) => State.t =
  (s, a) =>
    switch (a) {
    | Actions.Tick(_) => s
    | a =>
      let s = {
        ...s,
        buffers: Buffers.reduce(s.buffers, a),
        editorGroups: EditorGroups.reduce(s.editorGroups, a),
        lifecycle: Lifecycle.reduce(s.lifecycle, a),
        searchHighlights: SearchHighlights.reduce(a, s.searchHighlights),
        statusBar: StatusBarReducer.reduce(s.statusBar, a),
        fileExplorer: FileExplorer.reduce(s.fileExplorer, a),
        notifications: Notifications.reduce(s.notifications, a),
      };

      switch (a) {
      | KeyBindingsSet(keyBindings) => {...s, keyBindings}
      | SetLanguageInfo(languageInfo) => {...s, languageInfo}
      | SetIconTheme(iconTheme) => {...s, iconTheme}
      | ChangeMode(m) => {...s, mode: m};
      | SetEditorFont(font) => {...s, editorFont: font}
      | EnableZenMode => {...s, zenMode: true}
      | DisableZenMode => {...s, zenMode: false}
      | SetTokenTheme(tokenTheme) => {...s, tokenTheme}
      | _ => s
      };
    };
