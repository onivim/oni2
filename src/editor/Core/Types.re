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

  let toOneBasedInt = (pos: t) =>
    switch (pos) {
    | ZeroBasedIndex(n) => n + 1
    | OneBasedIndex(n) => n
    };
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

module BufferPosition = {
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
        ~modified=false,
        ~version=0,
        (),
      ) => {
    filePath,
    fileType,
    bufType,
    id,
    hidden,
    modified,
    version,
  };
};

module BufferNotification = {
  [@deriving show({with_path: false})]
  type t = {
    bufferId: int,
    buffers: list(BufferMetadata.t),
  };
};

module BufferUpdate = {
  [@deriving (show({with_path: false}), yojson({strict: false, exn: true}))]
  type t = {
    id: int,
    startLine: int,
    endLine: int,
    lines: list(string),
    version: int,
  };

  let create = (~id=0, ~startLine, ~endLine, ~lines, ~version, ()) => {
    id,
    startLine,
    endLine,
    lines,
    version,
  };
};

module Tabline = {
  [@deriving show({with_path: false})]
  type t = {
    tab: int,
    name: string,
  };

  [@deriving show({with_path: false})]
  type tabs = list(t);
};

module TextChanged = {
  type t = {
    activeBufferId: int,
    modified: bool,
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
    | [@name "editorTextFocus"] EditorTextFocus;

  [@deriving show({with_path: false})]
  type keyBindings = {
    key: string,
    command: string,
  };
};

module Effects = {
  /**
     dispatch: takes an 'a as the Action type is not known within
     this module so can only be specified at the point where this is
     used
   */
  [@deriving show({with_path: false})]
  type t('a) = {
    openFile: Views.viewOperation,
    getCurrentDir: unit => option(string),
    dispatch: 'a => unit,
    ripgrep: Ripgrep.t,
  };
};

module UiMenu = {
  [@deriving show({with_path: false})]
  type menuType =
    | QuickOpen
    | CommandPalette
    | Closed;

  [@deriving show({with_path: false})]
  type command = {
    name: string,
    command: unit => unit,
    icon: option(string),
  };

  type commandFactory('a) = Effects.t('a) => list(command);

  type t('a) = {
    effects: option(Effects.t('a)),
    searchQuery: string,
    menuType,
    isOpen: bool,
    commands: list(command),
    selectedItem: int,
  };
};
