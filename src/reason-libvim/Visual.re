open EditorCoreTypes;

let zeroRange = ByteRange.{start: BytePosition.zero, stop: BytePosition.zero};

let getRange = () => {
  let (startLine, startColumn, stopLine, stopColumn) =
    Native.vimVisualGetRange();

  // If there is no active range, startLine & stopLine will be '0'
  if (startLine == 0 || stopLine == 0) {
    zeroRange;
  } else {
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
    };
  };
};

let getType = Native.vimVisualGetType;
