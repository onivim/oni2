module Index = {
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
  type t = {
    pixelWidth: int,
    pixelHeight: int,
  };

  let create = (~pixelWidth: int, ~pixelHeight: int, ()) => {
    pixelWidth,
    pixelHeight,
  };
};

module Mode = {
  type t =
    | Insert
    | Normal
    | Commandline
    | Other;

  let show = v =>
    switch (v) {
    | Insert => "insert"
    | Normal => "normal"
    | Commandline => "commandline"
    | Other => "unknown"
    };
};

module BufferPosition = {
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

module ViewType = {
  type t =
    | Window
    | Tab
    | Buffer
    | Unknown;

  let getType = (enumVal): t => {
    switch (enumVal) {
    | 1 => Buffer
    | 2 => Tab
    | 3 => Window
    | _ => Unknown
    };
  };
};

type buffer = {
  filepath: string,
  id: int,
};

module BufferEnter = {
  type t = {
    bufferId: int,
    buffers: list(buffer),
  };
};

module BufferUpdate = {
  type t = {
    startLine: int,
    endLine: int,
    lines: list(string),
  };

  let create = (~startLine, ~endLine, ~lines, ()) => {
    startLine,
    endLine,
    lines,
  };
};

module Tabline = {
  type t = {
    tab: int,
    name: string,
  };

  type tabs = list(t);
};

module EditorFont = {
  type t = {
    fontFile: string,
    fontSize: int,
    measuredWidth: int,
    measuredHeight: int,
  };

  let create = (~fontFile, ~fontSize, ~measuredWidth, ~measuredHeight, ()) => {
    fontFile,
    fontSize,
    measuredWidth,
    measuredHeight,
  };
};

module Wildmenu = {
  type t = {
    items: list(string),
    show: bool,
    selected: int,
  };
};

module Commandline = {
  type t = {
    content: string,
    firstC: string,
    position: int,
    level: int,
    indent: int,
    prompt: string,
    show: bool,
  };
};
