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
