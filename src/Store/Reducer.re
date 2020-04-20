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
        definition: DefinitionReducer.reduce(a, s.definition),
        editorGroups:
          EditorGroups.reduce(~defaultFont=s.editorFont, s.editorGroups, a),
        extensions: ExtensionsReducer.reduce(a, s.extensions),
        languageFeatures:
          LanguageFeaturesReducer.reduce(a, s.languageFeatures),
        lifecycle: Lifecycle.reduce(s.lifecycle, a),
        sideBar: SideBarReducer.reduce(s.sideBar, a),
        statusBar: StatusBarReducer.reduce(s.statusBar, a),
      };

      switch (a) {
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
      | TokenThemeLoaded(tokenTheme) => {...s, tokenTheme}
      | EnableZenMode => {...s, zenMode: true}
      | DisableZenMode => {...s, zenMode: false}
      | ReallyQuitting => {...s, isQuitting: true}
      | WindowFocusGained => {...s, windowIsFocused: true}
      | WindowFocusLost => {...s, windowIsFocused: false}
      | WindowMaximized => {...s, windowIsMaximized: true}
      | WindowRestored
      | WindowMinimized => {...s, windowIsMaximized: false}
      | _ => s
      };
    };
