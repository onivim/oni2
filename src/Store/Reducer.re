/*
 * Reducer.re
 *
 * Manage state transitions based on Actions
 */

open Oni_Model;

let reduce: (State.t, Actions.t) => State.t =
  (s, a) =>
    switch (a) {
    | a =>
      let s = {
        ...s,
        bufferHighlights:
          BufferHighlightsReducer.reduce(s.bufferHighlights, a),
        bufferRenderers: BufferRendererReducer.reduce(s.bufferRenderers, a),
        languageFeatures:
          LanguageFeaturesReducer.reduce(a, s.languageFeatures),
        lifecycle: Lifecycle.reduce(s.lifecycle, a),
        sideBar: SideBarReducer.reduce(~zenMode=s.zenMode, s.sideBar, a),
      };

      switch (a) {
      // Turn off zenMode with :vsp/:sp
      | OpenFileByPath(_, Some(_), _) => {...s, zenMode: false}
      | KeyBindingsSet(keyBindings) => {...s, keyBindings}
      | SetLanguageInfo(languageInfo) => {...s, languageInfo}
      | SetGrammarRepository(grammarRepository) => {...s, grammarRepository}
      | SetIconTheme(iconTheme) => {...s, iconTheme}
      | TokenThemeLoaded(tokenTheme) => {...s, tokenTheme}
      | ActivityBar(ActivityBar.FileExplorerClick) => {...s, zenMode: false}
      | ActivityBar(ActivityBar.SCMClick) => {...s, zenMode: false}
      | ActivityBar(ActivityBar.ExtensionsClick) => {...s, zenMode: false}
      | ActivityBar(ActivityBar.SearchClick) => {...s, zenMode: false}
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
