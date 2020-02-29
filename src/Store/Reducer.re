/*
 * Reducer.re
 *
 * Manage state transitions based on Actions
 */

open Oni_Model;

module Diagnostics = Feature_LanguageSupport.Diagnostics;

let reduce: (State.t, Actions.t) => State.t =
  (s, a) =>
    switch (a) {
    | a =>
      let s = {
        ...s,
        buffers: Buffers.reduce(s.buffers, a),
        /*syntaxHighlights:
          BufferSyntaxHighlightsReducer.reduce(s.syntaxHighlights, a),*/
        bufferHighlights:
          BufferHighlightsReducer.reduce(s.bufferHighlights, a),
        bufferRenderers: BufferRendererReducer.reduce(s.bufferRenderers, a),
        commands: Commands.reduce(s.commands, a),
        definition: DefinitionReducer.reduce(a, s.definition),
        editorGroups: EditorGroups.reduce(s.editorGroups, a),
        extensions: ExtensionsReducer.reduce(a, s.extensions),
        languageFeatures:
          LanguageFeaturesReducer.reduce(a, s.languageFeatures),
        lifecycle: Lifecycle.reduce(s.lifecycle, a),
        sideBar: SideBarReducer.reduce(s.sideBar, a),
        statusBar: StatusBarReducer.reduce(s.statusBar, a),
        notifications: Notifications.reduce(s.notifications, a),
      };

      (
        switch (a) {
        | DarkModeSet(darkMode) => {...s, darkMode}
        | DiagnosticsSet(buffer, key, diags) => {
            ...s,
            diagnostics:
              Diagnostics.change(s.diagnostics, buffer, key, diags),
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
        | EditorFont(Service_Font.FontLoaded(font)) => {
            ...s,
            editorFont: font,
          }
        | TerminalFont(Service_Font.FontLoaded(font)) => {
            ...s,
            terminalFont: font,
          }
        | EnableZenMode => {...s, zenMode: true}
        | DisableZenMode => {...s, zenMode: false}
        | ReallyQuitting => {...s, isQuitting: true}
        | SetTokenTheme(tokenTheme) => {...s, tokenTheme}
        | WindowFocusGained => {...s, windowIsFocused: true}
        | WindowFocusLost => {...s, windowIsFocused: false}
        | WindowMaximized => {...s, windowIsMaximized: true}
        | WindowRestored
        | WindowMinimized => {...s, windowIsMaximized: false}
        | _ => s
        }
      )
      |> PaneReducer.reduce(a);
    };
