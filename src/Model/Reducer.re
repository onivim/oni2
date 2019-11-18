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
        completions: Completions.reduce(s.completions, a),
        editorGroups: EditorGroups.reduce(s.editorGroups, a),
        lifecycle: Lifecycle.reduce(s.lifecycle, a),
        searchHighlights: SearchHighlights.reduce(a, s.searchHighlights),
        statusBar: StatusBarReducer.reduce(s.statusBar, a),
        fileExplorer: FileExplorer.reduce(s.fileExplorer, a),
        notifications: Notifications.reduce(s.notifications, a),
        workspace: Workspace.reduce(s.workspace, a),
      };

      switch (a) {
      | DarkModeSet(darkMode) => {...s, darkMode}
      | DiagnosticsSet(buffer, key, diags) => {
          ...s,
          diagnostics: Diagnostics.change(s.diagnostics, buffer, key, diags),
        }
      | DiagnosticsClear(key) => {
          ...s,
          diagnostics: Diagnostics.clear(s.diagnostics, key),
        }
      | KeyBindingsSet(keyBindings) => {...s, keyBindings}
      | SetLanguageInfo(languageInfo) => {...s, languageInfo}
      | SetIconTheme(iconTheme) => {...s, iconTheme}
      | SetColorTheme(theme) => {...s, theme}
      | ChangeMode(m) => {...s, mode: m}
      | SetEditorFont(font) => {...s, editorFont: font}
      | EnableZenMode => {...s, zenMode: true}
      | DisableZenMode => {...s, zenMode: false}
      | SetTokenTheme(tokenTheme) => {...s, tokenTheme}
      | _ => s
      };
    };
