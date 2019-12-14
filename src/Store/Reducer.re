/*
 * Reducer.re
 *
 * Manage state transitions based on Actions
 */

open Oni_Model;

let reduce: (State.t, Actions.t) => State.t =
  (s, a) =>
    switch (a) {
    | Actions.Tick(_) => s
    | a =>
      let s = {
        ...s,
        buffers: Buffers.reduce(s.buffers, a),
        bufferSyntaxHighlights:
          BufferSyntaxHighlightsReducer.reduce(s.bufferSyntaxHighlights, a),
        bufferHighlights:
          BufferHighlightsReducer.reduce(s.bufferHighlights, a),
        bufferRenderers:
          BufferRendererReducer.reduce(s.bufferRenderers, a),
        commands: Commands.reduce(s.commands, a),
        completions: Completions.reduce(s.completions, a),
        definition: DefinitionReducer.reduce(a, s.definition),
        editorGroups: EditorGroups.reduce(s.editorGroups, a),
        extensions: Extensions.reduce(a, s.extensions),
        languageFeatures:
          LanguageFeaturesReducer.reduce(a, s.languageFeatures),
        lifecycle: Lifecycle.reduce(s.lifecycle, a),
        statusBar: StatusBarReducer.reduce(s.statusBar, a),
        notifications: Notifications.reduce(s.notifications, a),
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
