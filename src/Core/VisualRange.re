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
let _normalizeRange = (range: Range.t) =>
  if (range.start.line > range.stop.line
      || range.start.line == range.stop.line
      && range.start.column > range.stop.column) {
    Range.{start: range.stop, stop: range.start};
  } else {
    range;
  };

let create = (~mode, range) => {range: _normalizeRange(range), mode};
