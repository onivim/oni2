module Position = {
  type t =
    | ZeroBasedPosition(int)
    | OneBasedPosition(int);

  let toZeroBasedIndex = (pos: t) =>
    switch (pos) {
    | ZeroBasedPosition(n) => n
    | OneBasedPosition(n) => n - 1
    };
};

module Mode = {
  type t =
    | Insert
    | Normal
    | Other;

  let show = v =>
    switch (v) {
    | Insert => "insert"
    | Normal => "normal"
    | Other => "unknown"
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
