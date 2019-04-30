PositionToVirtualmodule Index = {
  [@deriving show({with_path: false})]
  type t =
    | ZeroBasedIndex(int)
    | OneBasedIndex(int);

  let toZeroBasedInt = (pos: t) =>
    switch (pos) {
    | ZeroBasedIndex(n) => n
    | OneBasedIndex(n) => n - 1
    };

  let ofInt0 = v => ZeroBasedIndex(v);
  let ofInt1 = v => OneBasedIndex(v);

  let toInt0 = toZeroBasedInt;

  let toOneBasedInt = (pos: t) =>
    switch (pos) {
    | ZeroBasedIndex(n) => n + 1
    | OneBasedIndex(n) => n
    };

  let toInt1 = toOneBasedInt;

  let zero = ZeroBasedIndex(0);
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

module Mode = {
  /**
     hide path for this printer as the value is shown
     to the end user
   */
  [@deriving show({with_path: false})]
  type t =
    | Insert
    | Normal
    | Replace
    | Visual
    | Operator
    | Commandline
    | Other;
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

  let fromIndices0 = createFromZeroBasedIndices;

  let toIndices0 = (v: t) => (
    Index.toInt0(v.line),
    Index.toInt0(v.character),
  );

  let zero = fromIndices0(0, 0);
};

[@deriving show({with_path: false})]
type buftype =
  | Empty
  | Help
  | NoFile
  | QuickFix
  | Terminal
  | NoWrite
  | ACWrite
  | Unknown;

let getBufType = bt =>
  switch (bt) {
  | "help" => Help
  | "nofile" => NoFile
  | "quickfix" => QuickFix
  | "terminal" => Terminal
  | "nowrite" => NoWrite
  | "acwrite" => ACWrite
  | "" => Empty
  | _ => Unknown
  };

module BufferMetadata = {
  [@deriving show({with_path: false})]
  type t = {
    filePath: option(string),
    fileType: option(string),
    bufType: buftype,
    modified: bool,
    hidden: bool,
    id: int,
    version: int,
  };

  let create =
      (
        ~filePath=None,
        ~fileType=None,
        ~bufType=Empty,
        ~id=0,
        ~hidden=false,
        ~version=0,
        ~modified=false,
        (),
      ) => {
    filePath,
    fileType,
    bufType,
    id,
    hidden,
    version,
    modified,
  };

  let markSaved = (bm: t) => {...bm, modified: false};

  let markDirty = (bm: t) => {...bm, modified: true};
};

module BufferUpdate = {
  [@deriving show({with_path: false})]
  type t = {
    id: int,
    startLine: Index.t,
    endLine: Index.t,
    lines: list(string),
    version: int,
  };

  [@deriving
    (show({with_path: false}), yojson({strict: false, exn: false}))
  ]
  type jsont = {
    id: int,
    startLine: int,
    endLine: int,
    lines: list(string),
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
    id,
    startLine,
    endLine,
    lines,
    version,
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

[@deriving show({with_path: false})]
type wildmenu = {
  items: list(string),
  show: bool,
  selected: int,
};

[@deriving show({with_path: false})]
type commandline = {
  content: string,
  firstC: string,
  position: int,
  level: int,
  indent: int,
  prompt: string,
  show: bool,
};

module Input = {
  [@deriving
    (show({with_path: false}), yojson({strict: false, exn: false}))
  ]
  type controlMode =
    | [@name "menuFocus"] MenuFocus
    | [@name "textInputFocus"] TextInputFocus
    | [@name "editorTextFocus"] EditorTextFocus
    | [@name "neovimMenuFocus"] NeovimMenuFocus;

  [@deriving show({with_path: false})]
  type keyBindings = {
    key: string,
    command: string,
  };
};
