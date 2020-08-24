open EditorCoreTypes;

type mode = Vim.Types.visualType;

type t = {
  range: ByteRange.t,
  mode,
};

/*
 * The range might not always come through in the correct 'order' -
 * this method normalizes the range so that the (startLine, startColumn) is
 * before or equal to (endLine, endColumn)
 */
let _normalizeRange = (range: ByteRange.t) =>
  if (LineNumber.(range.start.line > range.stop.line)
      || LineNumber.(range.start.line == range.stop.line)
      && ByteIndex.(range.start.byte > range.stop.byte)) {
    ByteRange.{start: range.stop, stop: range.start};
  } else {
    range;
  };

let create = (~mode, range) => {range: _normalizeRange(range), mode};
