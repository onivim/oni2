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
    | CommandlineShow(commandline) => {...s, commandline}
    | CommandlineHide(commandline) => {...s, commandline}
    | WildmenuShow(wildmenu) =>
      print_endline(
        "wildmenu.selected =======================: "
        ++ string_of_int(wildmenu.selected),
      );
      {...s, wildmenu};
    | WildmenuHide(wildmenu) => {...s, wildmenu}
    | Noop => s
    };
  };
