type t =
  | Goto(Goto.effect)
  | FormatBuffer({
      bufferId: int,
      adjustCursor: bool,
    })
  | FormatRange({
      bufferId: int,
      startLine: Index.t,
      endLine: Index.t,
      adjustCursor: bool,
    });
