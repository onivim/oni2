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
