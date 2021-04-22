open EditorCoreTypes;

type formatType =
  | Indentation
  | Formatting;

type effect =
  | Buffer({
      formatType,
      bufferId: int,
      adjustCursor: bool,
    })
  | Range({
      formatType,
      bufferId: int,
      // The inclusive startline of the format
      startLine: LineNumber.t,
      // The inclusive endline of the format
      endLine: LineNumber.t,
      adjustCursor: bool,
    });
