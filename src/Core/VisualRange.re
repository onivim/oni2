open EditorCoreTypes;

type mode = Vim.Types.visualType;

type t = {
  range: Range.t,
  mode,
};

/*
 * The range might not always come through in the correct 'order' -
 * this method normalizes the range so that the (startLine, startColumn) is
 * before or equal to (endLine, endColumn)
 */
let _normalizeRange = (startLine, startColumn, endLine, endColumn) =>
  if (startLine > endLine) {
    (endLine, endColumn, startLine, startColumn);
  } else if (startLine == endLine && startColumn > endColumn) {
    (endLine, endColumn, startLine, startColumn);
  } else {
    (startLine, startColumn, endLine, endColumn);
  };

let create =
    (
      ~startLine=1,
      ~startColumn=1,
      ~endLine=1,
      ~endColumn=1,
      ~mode=Vim.Types.None,
      (),
    ) => {
  let (startLine, startColumn, endLine, endColumn) =
    _normalizeRange(startLine, startColumn, endLine, endColumn);

  let range =
    Range.create(
      ~start=
        Location.create(
          ~line=Index.fromOneBased(startLine),
          ~column=Index.fromOneBased(startColumn),
        ),
      ~stop=
        Location.create(
          ~line=Index.fromOneBased(endLine),
          ~column=Index.fromOneBased(endColumn),
        ),
    );

  {range, mode};
};
