open EditorCoreTypes;

let zeroRange =
  CharacterRange.{
    start: CharacterPosition.zero,
    stop: CharacterPosition.zero
  };

let getRange = () => {
  let (startLine, startColumn, stopLine, stopColumn) =
    Native.vimVisualGetRange();

  // If there is no active range, startLine & stopLine will be '0'
  if (startLine == 0 || stopLine == 0) {
    zeroRange;
  } else {
    CharacterRange.{
      start: CharacterPosition.{
        line: LineNumber.ofOneBased(startLine),
        character: CharacterIndex.ofInt(startColumn),
      },
      stop: CharacterPosition.{
        line: LineNumber.ofOneBased(stopLine),
        character: CharacterIndex.ofInt(stopColumn),
      }
    }
  };
};

let getType = Native.vimVisualGetType;

let onRangeChanged = f => {
  Event.add(f, Listeners.visualRangeChanged);
};
