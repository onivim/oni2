/*
 * Reducer.re
 *
 * Manage state transitions based on Actions
 */

open Actions;

let reduce: (State.t, Actions.t) => State.t =
  (s, a) => {
    switch (a) {
    | ChangeMode(m) =>
      let ret: State.t = {...s, mode: m};
      ret;
    | CursorMove(b) => {...s, cursorPosition: b}
    | BufferUpdate(bu) => {...s, buffer: Buffer.update(s.buffer, bu)}
    | SetEditorFont(font) => {...s, editorFont: font}
    | SetEditorSize(size) => {...s, size}
    | CommandlineShow(commandline) => {...s, commandline}
    | CommandlineHide(commandline) => {...s, commandline}
    | Noop => s
    };
  };
