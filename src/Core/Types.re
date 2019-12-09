module Index = {
  [@deriving show({with_path: false})]
  type t =
    | ZeroBasedIndex(int)
    | OneBasedIndex(int);

  let toZeroBasedInt = (pos: t) =>
    switch (pos) {
    | ZeroBasedIndex(n) => n
    | OneBasedIndex(n) => n - 1
    };

  let toInt0 = toZeroBasedInt;

  let toOneBasedInt = (pos: t) =>
    switch (pos) {
    | ZeroBasedIndex(n) => n + 1
    | OneBasedIndex(n) => n
    };

  let toInt1 = toOneBasedInt;

  let ofInt0 = i => ZeroBasedIndex(i);
  let ofInt1 = i => OneBasedIndex(i);

  let equals = (a: t, b: t) => toInt0(a) == toInt0(b);
};

module EditorSize = {
  [@deriving show({with_path: false})]
  type t = {
    pixelWidth: int,
    pixelHeight: int,
  };

  let create = (~pixelWidth: int, ~pixelHeight: int, ()) => {
    pixelWidth,
    pixelHeight,
  };
};

module Cursor = {
  type move = (~column: int, ~line: int) => unit;
};

/**
   This type module represents the various "views" of vim
   each of which can be interacted in their own ways

   * A Tab: contains windows
   * A Window: contains buffers
   * A buffer: represents a file
 */
module Views = {
  [@deriving show({with_path: false})]
  type t =
    | Window
    | Tab
    | Buffer;

  type openMethod =
    | Buffer
    | Tab;

  [@deriving show({with_path: false})]
  type viewOperation =
    (~path: string=?, ~id: int=?, ~openMethod: openMethod=?, unit) => unit;
};

type openMethod =
  | Tab
  | Buffer;

module Position = {
  [@deriving show({with_path: false})]
  type t = {
    line: Index.t,
    character: Index.t,
  };

  let create = (line, character) => {line, character};

  let createFromZeroBasedIndices = (line: int, character: int) => {
    line: ZeroBasedIndex(line),
    character: ZeroBasedIndex(character),
  };

  let ofInt0 = createFromZeroBasedIndices;

  let createFromOneBasedIndices = (line: int, character: int) => {
    line: OneBasedIndex(line),
    character: OneBasedIndex(character),
  };

  let ofInt1 = createFromOneBasedIndices;

  let equals = (a: t, b: t) => {
    Index.equals(a.line, b.line) && Index.equals(a.character, b.character);
  };
};

module EditorFont = {
  module Zed_utf8 = ZedBundled;

  type t = {
    fontFile: string,
    fontSize: int,
    measuredWidth: float,
    measuredHeight: float,
  };

  let getHeight = (v: t) => v.measuredHeight;

  let measure = (~text, v: t) => {
    float_of_int(Zed_utf8.length(text)) *. v.measuredWidth;
  };

  let create = (~fontFile, ~fontSize, ~measuredWidth, ~measuredHeight, ()) => {
    fontFile,
    fontSize,
    measuredWidth,
    measuredHeight,
  };
};

module UiFont = {
  type t = {
    fontFile: string,
    fontSize: int,
  };

  let create = (~fontFile, ~fontSize, ()) => {fontFile, fontSize};
};

module Input = {
  [@deriving
    (show({with_path: false}), yojson({strict: false, exn: false}))
  ]
  // This type is overloaded - describing both the current 'input mode'
  // the UI is in, as well as the state of 'when' conditions in the input
  // bindings. Need to decouple these.
  type controlMode =
    // VSCode-compatible when parameters
    | [@name "listFocus"] ListFocus
    | [@name "textInputFocus"] TextInputFocus
    | [@name "inQuickOpen"] InQuickOpen
    | [@name "editorTextFocus"] EditorTextFocus
    | [@name "suggestWidgetVisible"] SuggestWidgetVisible
    // Onivim extensions to the 'when' syntax
    | [@name "insertMode"] InsertMode
    | [@name "quickmenuCursorEnd"] QuickmenuCursorEnd;

  [@deriving show({with_path: false})]
  type keyBindings = {
    key: string,
    command: string,
  };
};
module ColorizedToken = {
  type t = {
    index: int,
    backgroundColor: Revery.Color.t,
    foregroundColor: Revery.Color.t,
  };

  let create = (~index, ~backgroundColor, ~foregroundColor, ()) => {
    index,
    backgroundColor,
    foregroundColor,
  };

  let show = v =>
    Printf.sprintf(
      "index: %d bg: %s fg: %s",
      v.index,
      Revery.Color.toString(v.backgroundColor),
      Revery.Color.toString(v.foregroundColor),
    );
};
