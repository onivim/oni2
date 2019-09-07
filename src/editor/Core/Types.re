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
};

module BufferUpdate = {
  [@deriving show({with_path: false})]
  type t = {
    id: int,
    startLine: Index.t,
    endLine: Index.t,
    lines: array(string),
    version: int,
  };

  [@deriving
    (show({with_path: false}), yojson({strict: false, exn: false}))
  ]
  type jsont = {
    id: int,
    startLine: int,
    endLine: int,
    lines: array(string),
    version: int,
  };

  let to_yojson = (v: t) => {
    let jsonr: jsont = {
      id: v.id,
      startLine: v.startLine |> Index.toZeroBasedInt,
      endLine: v.endLine |> Index.toZeroBasedInt,
      lines: v.lines,
      version: v.version,
    };
    jsont_to_yojson(jsonr);
  };

  let create = (~id=0, ~startLine, ~endLine, ~lines, ~version, ()) => {
    let ret: t = {id, startLine, endLine, lines, version};
    ret;
  };
  let createFromZeroBasedIndices =
      (~id=0, ~startLine: int, ~endLine: int, ~lines, ~version, ()) => {
    let ret: t = {
      id,
      startLine: Index.ZeroBasedIndex(startLine),
      endLine: Index.ZeroBasedIndex(endLine),
      lines,
      version,
    };
    ret;
  };
  let createFromOneBasedIndices =
      (~id=0, ~startLine: int, ~endLine: int, ~lines, ~version, ()) => {
    let ret: t = {
      id,
      startLine: Index.OneBasedIndex(startLine),
      endLine: Index.OneBasedIndex(endLine),
      lines,
      version,
    };
    ret;
  };
};

module EditorFont = {
  type t = {
    fontFile: string,
    fontSize: int,
    measuredWidth: float,
    measuredHeight: float,
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

/* [@deriving show({with_path: false})] */
type commandline = {
  text: string,
  cmdType: Vim.Types.cmdlineType,
  position: int,
  show: bool,
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
    | [@name "menuFocus"] MenuFocus
    | [@name "textInputFocus"] TextInputFocus
    | [@name "editorTextFocus"] EditorTextFocus
    | [@name "commandLineFocus"] CommandLineFocus
    // Onivim extensions to the 'when' syntax
    | [@name "insertMode"] InsertMode;

  [@deriving show({with_path: false})]
  type keyBindings = {
    key: string,
    command: string,
  };
};

// TEMPORARY representation of token colors, while we are
// migrating from the legacy node-based strategy to the new
// native strategy.
module ColorizedToken2 = {
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
};
