open EditorCoreTypes;

let getHighlightsInRange = (startLine, stopLine) => {
  let highlights = Native.vimSearchGetHighlights(startLine, stopLine);
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

let getHighlights = () => {
  getHighlightsInRange(0, 0);
};

let onStopSearchHighlight = f => Event.add(f, Listeners.stopSearchHighlight);
