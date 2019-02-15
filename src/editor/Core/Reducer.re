/*
 * Reducer.re
 *
 * Manage state transitions based on Actions
 */

open Actions;

let updateBufferList = (_next, prev) => {
  prev;
};

let reduce: (State.t, Actions.t) => State.t =
  (s, a) => {
    switch (a) {
    | ChangeMode(m) =>
      let ret: State.t = {...s, mode: m};
      ret;
    | CursorMove(b) => {...s, cursorPosition: b}
    | BufferEnter(b) => {
        ...s,
        activeBufferId: b.bufferId,
        buffers: updateBufferList(b.buffers, s.buffers),
      }
    | BufferUpdate(bu) => {
        ...s,
        activeBuffer: Buffer.update(s.activeBuffer, bu),
      }
    | SetEditorFont(font) => {...s, editorFont: font}
    | SetEditorSize(size) => {...s, size}
    | CommandlineShow(commandline) => {...s, commandline}
    | CommandlineHide(commandline) => {...s, commandline}
    | WildmenuShow(wildmenu) => {...s, wildmenu}
    | WildmenuHide(wildmenu) => {...s, wildmenu}
    | WildmenuSelected(selected) => {
        ...s,
        wildmenu: {
          ...s.wildmenu,
          selected,
        },
      }
    | Noop => s
    };
  };
