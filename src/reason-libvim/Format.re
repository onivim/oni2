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
      requestType: formatType,
      bufferId: int,
      startLine: Index.t,
      endLine: Index.t,
      adjustCursor: bool,
    });
