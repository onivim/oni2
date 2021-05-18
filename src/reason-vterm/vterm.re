type terminal;

type modifier =
  | None
  | Shift
  | Alt
  | Control
  | All;

type key =
  | Unicode(Uchar.t)
  | Enter
  | Tab
  | Backspace
  | Escape
  | Up
  | Down
  | Left
  | Right
  | Insert
  | Delete
  | Home
  | End
  | PageUp
  | PageDown;

type size = {
  rows: int,
  cols: int,
};

module Rect = {
  type t = {
    startRow: int,
    endRow: int,
    startCol: int,
    endCol: int,
  };

  let toString = ({startRow, endRow, startCol, endCol}) =>
    Printf.sprintf(
      "startRow: %d startCol: %d endRow: %d endCol: %d",
      startRow,
      startCol,
      endRow,
      endCol,
    );
};

module Pos = {
  type t = {
    row: int,
    col: int,
  };

  let toString = ({row, col}) =>
    Printf.sprintf("row: %d col: %d", row, col);
};

module TermProp = {
  module CursorShape = {
    type t =
      | Block
      | Underline
      | BarLeft
      | Unknown;

    let toString =
      fun
      | Block => "Block"
      | Underline => "Underline"
      | BarLeft => "BarLeft"
      | Unknown => "Unknown";
  };

  module Mouse = {
    type t =
      | None
      | Click
      | Drag
      | Move;

    let toString =
      fun
      | None => "None"
      | Click => "Click"
      | Drag => "Drag"
      | Move => "Move";
  };

  type t =
    | None
    | CursorVisible(bool)
    | CursorBlink(bool)
    | AltScreen(bool)
    | Title(string)
    | IconName(string)
    | Reverse(bool)
    | CursorShape(CursorShape.t)
    | Mouse(Mouse.t);

  let toString =
    fun
    | None => "None"
    | CursorVisible(viz) => Printf.sprintf("CursorVisible: %b", viz)
    | CursorBlink(blink) => Printf.sprintf("CursorBlink: %B", blink)
    | AltScreen(alt) => Printf.sprintf("AltScreen: %b", alt)
    | Title(str) => Printf.sprintf("Title: %s", str)
    | IconName(str) => Printf.sprintf("IconName: %s", str)
    | Reverse(rev) => Printf.sprintf("Reverse: %b", rev)
    | CursorShape(cursor) =>
      Printf.sprintf("CursorShape: %s", CursorShape.toString(cursor))
    | Mouse(mouse) => Printf.sprintf("Mouse: %s", Mouse.toString(mouse));
};

module Color = {
  type raw = int;

  type t =
    | DefaultForeground
    | DefaultBackground
    | Rgb(int, int, int)
    | Index(int);

  let defaultForeground = 1024;
  let defaultBackground = 1025;

  let toString =
    fun
    | DefaultForeground => "DefaultForeground"
    | DefaultBackground => "DefaultBackground"
    | Rgb(r, g, b) => Printf.sprintf("rgb(%d, %d, %d)", r, g, b)
    | Index(idx) => Printf.sprintf("index(%d)", idx);

  let unpack = raw => {
    let controlBit = raw land 3;
    switch (controlBit) {
    | 0 => DefaultBackground
    | 1 => DefaultForeground
    | 2 =>
      let r = (raw land 255 lsl 18) lsr 18;
      let g = (raw land 255 lsl 10) lsr 10;
      let b = (raw land 255 lsl 2) lsr 2;
      Rgb(r, g, b);
    | 3 =>
      let idx = (raw land 255 lsl 2) lsr 2;
      Index(idx);
    | _ => DefaultForeground
    };
  };
};

module Style = {
  type t = int;

  let isBold = v => v land 1 == 1;
  let isItalic = v => v land 2 == 2;
  let isUnderline = v => v land 4 == 4;
};

module ScreenCell = {
  type t = {
    char: Uchar.t,
    fg: Color.raw,
    bg: Color.raw,
    style: Style.t,
  };

  let empty: t = {
    char: Uchar.of_int(0),
    fg: Color.defaultForeground,
    bg: Color.defaultBackground,
    style: 0,
  };
};

type callbacks = {
  onTermOutput: ref(string => unit),
  onScreenDamage: ref(Rect.t => unit),
  onScreenMoveRect: ref((Rect.t, Rect.t) => unit),
  onScreenMoveCursor: ref((Pos.t, Pos.t, bool) => unit),
  onScreenSetTermProp: ref(TermProp.t => unit),
  onScreenBell: ref(unit => unit),
  onScreenResize: ref(size => unit),
  onScreenScrollbackPushLine: ref(array(ScreenCell.t) => unit),
  onScreenScrollbackPopLine: ref(array(ScreenCell.t) => unit),
};

type t = {
  uniqueId: int,
  terminal,
  callbacks,
};

let idToOutputCallback: Hashtbl.t(int, callbacks) = Hashtbl.create(8);

module Internal = {
  let uniqueId = ref(0);

  external newVterm: (int, int, int) => terminal = "reason_libvterm_vterm_new";
  external freeVterm: terminal => unit = "reason_libvterm_vterm_free";
  external set_utf8: (terminal, bool) => unit =
    "reason_libvterm_vterm_set_utf8";
  external get_utf8: terminal => bool = "reason_libvterm_vterm_get_utf8";
  external get_size: terminal => size = "reason_libvterm_vterm_get_size";
  external set_size: (terminal, size) => unit =
    "reason_libvterm_vterm_set_size";
  external input_write: (terminal, string) => int =
    "reason_libvterm_vterm_input_write";

  external keyboard_unichar: (terminal, Int32.t, modifier) => unit =
    "reason_libvterm_vterm_keyboard_unichar";

  external keyboard_key: (terminal, key, modifier) => unit =
    "reason_libvterm_vterm_keyboard_key";

  external screen_get_cell: (terminal, int, int) => ScreenCell.t =
    "reason_libvterm_vterm_screen_get_cell";

  external screen_enable_altscreen: (terminal, int) => unit =
    "reason_libvterm_vterm_screen_enable_altscreen";

  let onOutput = (id: int, output: string) => {
    switch (Hashtbl.find_opt(idToOutputCallback, id)) {
    | Some({onTermOutput, _}) => onTermOutput^(output)
    | None => ()
    };
  };

  let onScreenBell = (id: int) => {
    switch (Hashtbl.find_opt(idToOutputCallback, id)) {
    | Some({onScreenBell, _}) => onScreenBell^()
    | None => ()
    };
  };

  let onScreenResize = (id: int, rows: int, cols: int) => {
    switch (Hashtbl.find_opt(idToOutputCallback, id)) {
    | Some({onScreenResize, _}) => onScreenResize^({rows, cols})
    | None => ()
    };
  };

  let onScreenDamage = (id: int, rect: Rect.t) => {
    switch (Hashtbl.find_opt(idToOutputCallback, id)) {
    | Some({onScreenDamage, _}) => onScreenDamage^(rect)
    | None => ()
    };
  };

  let onScreenMoveCursor = (id: int, newRow, newCol, oldRow, oldCol, visible) => {
    switch (Hashtbl.find_opt(idToOutputCallback, id)) {
    | Some({onScreenMoveCursor, _}) =>
      onScreenMoveCursor^(
        Pos.{row: newRow, col: newCol},
        Pos.{row: oldRow, col: oldCol},
        visible,
      )
    | None => ()
    };
  };

  let onScreenMoveRect =
      (
        id: int,
        destStartRow: int,
        destStartCol: int,
        destEndRow: int,
        destEndCol: int,
        srcStartRow: int,
        srcStartCol: int,
        srcEndRow: int,
        srcEndCol: int,
      ) => {
    switch (Hashtbl.find_opt(idToOutputCallback, id)) {
    | Some({onScreenMoveRect, _}) =>
      onScreenMoveRect^(
        Rect.{
          startRow: destStartRow,
          startCol: destStartCol,
          endRow: destEndRow,
          endCol: destEndCol,
        },
        Rect.{
          startRow: srcStartRow,
          startCol: srcStartCol,
          endRow: srcEndRow,
          endCol: srcEndCol,
        },
      )
    | None => ()
    };
  };

  let onScreenSetTermProp = (id: int, termProp: TermProp.t) => {
    switch (Hashtbl.find_opt(idToOutputCallback, id)) {
    | Some({onScreenSetTermProp, _}) => onScreenSetTermProp^(termProp)
    | None => ()
    };
  };

  let onScreenSbPushLine = (id: int, cells: array(ScreenCell.t)) => {
    switch (Hashtbl.find_opt(idToOutputCallback, id)) {
    | Some({onScreenScrollbackPushLine, _}) =>
      onScreenScrollbackPushLine^(cells)
    | None => ()
    };
  };

  let onScreenSbPopLine = (id: int, cells: array(ScreenCell.t)) => {
    switch (Hashtbl.find_opt(idToOutputCallback, id)) {
    | Some({onScreenScrollbackPopLine, _}) =>
      onScreenScrollbackPopLine^(cells)
    | None => ()
    };
  };

  Callback.register("reason_libvterm_onOutput", onOutput);
  Callback.register("reason_libvterm_onScreenBell", onScreenBell);
  Callback.register("reason_libvterm_onScreenResize", onScreenResize);
  Callback.register("reason_libvterm_onScreenDamage", onScreenDamage);
  Callback.register("reason_libvterm_onScreenMoveCursor", onScreenMoveCursor);
  Callback.register("reason_libvterm_onScreenMoveRect", onScreenMoveRect);
  Callback.register(
    "reason_libvterm_onScreenSetTermProp",
    onScreenSetTermProp,
  );
  Callback.register("reason_libvterm_onScreenSbPushLine", onScreenSbPushLine);
  Callback.register("reason_libvterm_onScreenSbPopLine", onScreenSbPopLine);
};

module Screen = {
  let setBellCallback = (~onBell, terminal) => {
    terminal.callbacks.onScreenBell := onBell;
  };

  let setResizeCallback = (~onResize, terminal) => {
    terminal.callbacks.onScreenResize := onResize;
  };

  let setDamageCallback = (~onDamage, terminal) => {
    terminal.callbacks.onScreenDamage := onDamage;
  };

  let setMoveCursorCallback = (~onMoveCursor, terminal) => {
    terminal.callbacks.onScreenMoveCursor := onMoveCursor;
  };

  let setMoveRectCallback = (~onMoveRect, terminal) => {
    terminal.callbacks.onScreenMoveRect := onMoveRect;
  };

  let setScrollbackPopCallback = (~onPopLine, terminal) => {
    terminal.callbacks.onScreenScrollbackPopLine := onPopLine;
  };

  let setScrollbackPushCallback = (~onPushLine, terminal) => {
    terminal.callbacks.onScreenScrollbackPushLine := onPushLine;
  };

  let getCell = (~row, ~col, {terminal, _}) => {
    Internal.screen_get_cell(terminal, row, col);
  };

  let setAltScreen = (~enabled, {terminal, _}) => {
    Internal.screen_enable_altscreen(terminal, enabled ? 1 : 0);
  };

  let setTermPropCallback = (~onSetTermProp, terminal) => {
    terminal.callbacks.onScreenSetTermProp := onSetTermProp;
  };
};

module Keyboard = {
  let input = ({terminal, _}, key, mods: modifier) => {
    switch (key) {
    | Unicode(uchar) =>
      let key = uchar |> Uchar.to_int |> Int32.of_int;
      Internal.keyboard_unichar(terminal, key, mods);
    | key => Internal.keyboard_key(terminal, key, mods)
    };
  };
};

let make = (~rows, ~cols) => {
  incr(Internal.uniqueId);
  let uniqueId = Internal.uniqueId^;
  let terminal = Internal.newVterm(uniqueId, rows, cols);
  let onTermOutput = ref(_ => ());
  let onScreenDamage = ref(_ => ());
  let onScreenMoveRect = ref((_, _) => ());
  let onScreenMoveCursor = ref((_, _, _) => ());
  let onScreenBell = ref(() => ());
  let onScreenResize = ref(_ => ());
  let onScreenSetTermProp = ref(_ => ());
  let onScreenScrollbackPushLine = ref(_ => ());
  let onScreenScrollbackPopLine = ref(_ => ());
  let callbacks = {
    onTermOutput,
    onScreenDamage,
    onScreenMoveRect,
    onScreenMoveCursor,
    onScreenSetTermProp,
    onScreenBell,
    onScreenResize,
    onScreenScrollbackPushLine,
    onScreenScrollbackPopLine,
  };
  let wrappedTerminal: t = {terminal, uniqueId, callbacks};
  Hashtbl.add(idToOutputCallback, uniqueId, callbacks);
  let () =
    Gc.finalise(
      ({terminal, uniqueId, _}: t) => {
        Internal.freeVterm(terminal);
        Hashtbl.remove(idToOutputCallback, uniqueId);
      },
      wrappedTerminal,
    );
  wrappedTerminal;
};

let setOutputCallback = (~onOutput, terminal) => {
  terminal.callbacks.onTermOutput := onOutput;
};

let setUtf8 = (~utf8, {terminal, _}) => {
  Internal.set_utf8(terminal, utf8);
};

let getUtf8 = ({terminal, _}) => {
  Internal.get_utf8(terminal);
};

let setSize = (~size, {terminal, _}) => {
  Internal.set_size(terminal, size);
};

let getSize = ({terminal, _}) => {
  Internal.get_size(terminal);
};

let write = (~input, {terminal, _}) => {
  Internal.input_write(terminal, input);
};
