type effect =
  | Buffer({
      bufferId: int,
      adjustCursor: bool,
    })
  | Range({
      bufferId: int,
      startLine: Index.t,
      endLine: Index.t,
      adjustCursor: bool,
    });
