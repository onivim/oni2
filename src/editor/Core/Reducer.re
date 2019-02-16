/*
 * Reducer.re
 *
 * Manage state transitions based on Actions
 */

open Actions;
open Types;

let updateTabline = tabs => {
  List.map(
    ({tab, name}: Tabline.t) =>
      State.Tab.{id: tab, title: name, active: false},
    tabs,
  );
};

let reduce: (State.t, Actions.t) => State.t =
  (s, a) => {
    switch (a) {
    | ChangeMode(m) =>
      let ret: State.t = {...s, mode: m};
      ret;
    | CursorMove(b) => {...s, cursorPosition: b}
    | BufferEnter(bs) => {
        ...s,
        activeBufferId: bs.bufferId,
        buffers: BufferList.update(s.buffers, bs.buffers),
      }
    | BufferUpdate(bu) => {
        ...s,
        activeBuffer: Buffer.update(s.activeBuffer, bu),
      }
    | TablineUpdate(tabs) => {...s, tabs: updateTabline(tabs)}
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
