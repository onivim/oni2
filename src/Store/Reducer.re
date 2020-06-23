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
        extensions: ExtensionsReducer.reduce(a, s.extensions),
        languageFeatures:
          LanguageFeaturesReducer.reduce(a, s.languageFeatures),
        lifecycle: Lifecycle.reduce(s.lifecycle, a),
        sideBar: SideBarReducer.reduce(~zenMode=s.zenMode, s.sideBar, a),
      };

      switch (a) {
      // Turn off zenMode with :vsp/:sp
      | OpenFileByPath(_, Some(_), _) => {...s, zenMode: false }
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
      | SetGrammarRepository(grammarRepository) => {...s, grammarRepository}
      | SetIconTheme(iconTheme) => {...s, iconTheme}
      | TokenThemeLoaded(tokenTheme) => {...s, tokenTheme}
      | ActivityBar(ActivityBar.FileExplorerClick) => {...s, zenMode: false}
      | ActivityBar(ActivityBar.SCMClick) => {...s, zenMode: false}
      | ActivityBar(ActivityBar.ExtensionsClick) => {...s, zenMode: false}
      | EnableZenMode => {...s, zenMode: true}
      | DisableZenMode => {...s, zenMode: false}
      | ReallyQuitting => {...s, isQuitting: true}
      | WindowFocusGained => {...s, windowIsFocused: true}
      | WindowFocusLost => {...s, windowIsFocused: false}
      | WindowMaximized => {...s, windowDisplayMode: Maximized}
      | WindowFullscreen => {...s, windowDisplayMode: Fullscreen}
      | WindowRestored
      | WindowMinimized => {...s, windowDisplayMode: Minimized}
      | _ => s
      };
    };
