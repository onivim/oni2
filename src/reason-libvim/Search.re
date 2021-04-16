open EditorCoreTypes;

let getHighlightsInRange = (buffer, startLine, stopLine) => {
  let highlights = Native.vimSearchGetHighlights(buffer, startLine, stopLine);
  Array.map(
    ((startLine, startColumn, stopLine, stopColumn)) =>
      ByteRange.{
        start:
          BytePosition.{
            line: LineNumber.ofOneBased(startLine),
            byte: ByteIndex.ofInt(startColumn),
          },
        stop:
          BytePosition.{
            line: LineNumber.ofOneBased(stopLine),
            byte: ByteIndex.ofInt(stopColumn),
          },
      },
    highlights,
  );
};

let getSearchPattern = () => Native.vimSearchGetPattern();

let getHighlights = buffer => {
  getHighlightsInRange(buffer, 0, 0);
};
