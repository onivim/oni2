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
        bufferRenderers: BufferRendererReducer.reduce(s.bufferRenderers, a),
        lifecycle: Lifecycle.reduce(s.lifecycle, a),
      };

      switch (a) {
      // Turn off zenMode with :vsp/:sp
      | OpenFileByPath(_, Some(_), _) => {
          ...s,
          zen: Feature_Zen.exitZenMode(s.zen),
        }
      | PreviewFileByPath(_, Some(_), _) => {
          ...s,
          zen: Feature_Zen.exitZenMode(s.zen),
        }
      | SetLanguageInfo(languageInfo) => {...s, languageInfo}
      | SetGrammarRepository(grammarRepository) => {...s, grammarRepository}
      | SetIconTheme(iconTheme) => {...s, iconTheme}
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
