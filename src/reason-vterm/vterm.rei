type t;

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

let make: (~rows: int, ~cols: int) => t;

let setOutputCallback: (~onOutput: string => unit, t) => unit;

let setUtf8: (~utf8: bool, t) => unit;
let getUtf8: t => bool;

let setSize: (~size: size, t) => unit;
let getSize: t => size;

let write: (~input: string, t) => int;

module Rect: {
  type t = {
    startRow: int,
    endRow: int,
    startCol: int,
    endCol: int,
  };

  let toString: t => string;
};

module Pos: {
  type t = {
    row: int,
    col: int,
  };

  let toString: t => string;
};

module TermProp: {
  module CursorShape: {
    type t =
      | Block
      | Underline
      | BarLeft
      | Unknown;

    let toString: t => string;
  };

  module Mouse: {
    type t =
      | None
      | Click
      | Drag
      | Move;

    let toString: t => string;
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

  let toString: t => string;
};

module Color: {
  type raw;

  type t =
    | DefaultForeground
    | DefaultBackground
    | Rgb(int, int, int)
    | Index(int);

  let unpack: raw => t;

  let toString: t => string;
};

module Style: {
  type t;

  let isBold: t => bool;
  let isUnderline: t => bool;
  let isItalic: t => bool;
};

module ScreenCell: {
  type t = {
    char: Uchar.t,
    fg: Color.raw,
    bg: Color.raw,
    // Attributes
    style: Style.t,
    // TODO:
    //    blink: int,
    //    reverse: int,
    //    conceal: int,
    //    strike: int,
    //font: int,
    //dwl: int,
    //dhl: int,
  };

  let empty: t;
};

module Screen: {
  let setBellCallback: (~onBell: unit => unit, t) => unit;
  let setResizeCallback: (~onResize: size => unit, t) => unit;
  let setDamageCallback: (~onDamage: Rect.t => unit, t) => unit;
  let setMoveCursorCallback:
    (~onMoveCursor: (Pos.t, Pos.t, bool) => unit, t) => unit;
  let setMoveRectCallback: (~onMoveRect: (Rect.t, Rect.t) => unit, t) => unit;
  let setTermPropCallback: (~onSetTermProp: TermProp.t => unit, t) => unit;

  let setScrollbackPopCallback:
    (~onPopLine: array(ScreenCell.t) => unit, t) => unit;
  let setScrollbackPushCallback:
    (~onPushLine: array(ScreenCell.t) => unit, t) => unit;

  let getCell: (~row: int, ~col: int, t) => ScreenCell.t;
  let setAltScreen: (~enabled: bool, t) => unit;
};

module Keyboard: {let input: (t, key, modifier) => unit;};
