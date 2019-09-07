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
        syntaxHighlighting:
          SyntaxHighlighting.reduce(s.syntaxHighlighting, a),
        wildmenu: Wildmenu.reduce(s.wildmenu, a),
        commandline: Commandline.reduce(s.commandline, a),
        searchHighlights: SearchHighlights.reduce(a, s.searchHighlights),
        statusBar: StatusBarReducer.reduce(s.statusBar, a),
        fileExplorer: FileExplorer.reduce(s.fileExplorer, a),
        notifications: Notifications.reduce(s.notifications, a),
      };

      switch (a) {
      | DiagnosticsSet(buffer, key, diags) => {
          ...s,
          diagnostics: Diagnostics.change(s.diagnostics, buffer, key, diags),
        }
      | SetLanguageInfo(languageInfo) => {...s, languageInfo}
      | SetIconTheme(iconTheme) => {...s, iconTheme}
      | ChangeMode(m) =>
        let ret: State.t = {...s, mode: m};
        ret;
      | SetEditorFont(font) => {...s, editorFont: font}
      | SetInputControlMode(m) => {...s, inputControlMode: m}
      | CommandlineShow(_) => {...s, inputControlMode: CommandLineFocus}
      | CommandlineHide => {...s, inputControlMode: EditorTextFocus}
      | EnableZenMode => {...s, zenMode: true}
      | DisableZenMode => {...s, zenMode: false}
      | SetTokenTheme(tokenTheme) => {...s, tokenTheme}
      | _ => s
      };
    };
