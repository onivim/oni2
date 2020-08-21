open EditorCoreTypes;

let getHighlightsInRange = (startLine, stopLine) => {
  let highlights = Native.vimSearchGetHighlights(startLine, stopLine);
  Array.map(
    ((startLine, startColumn, stopLine, stopColumn)) =>
      CharacterRange.{
        start:
          CharacterPosition.{
            line: LineNumber.ofOneBased(startLine),
            character: CharacterIndex.ofInt(startColumn),
          },
        stop:
          CharacterPosition.{
            line: LineNumber.ofOneBased(stopLine),
            character: CharacterIndex.ofInt(stopColumn),
          },
      },
    highlights,
  );
};

let getHighlights = () => {
  getHighlightsInRange(0, 0);
};

let onStopSearchHighlight = f => Event.add(f, Listeners.stopSearchHighlight);
