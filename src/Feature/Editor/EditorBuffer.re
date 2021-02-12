type t = Oni_Core.Buffer.t;

let ofBuffer = buffer => buffer;
let id = Oni_Core.Buffer.getId;
let getEstimatedMaxLineLength = Oni_Core.Buffer.getEstimatedMaxLineLength;
let numberOfLines = Oni_Core.Buffer.getNumberOfLines;
let line = Oni_Core.Buffer.getLine;
let font = Oni_Core.Buffer.getFont;

let fileType = Oni_Core.Buffer.getFileType;
let measure = Oni_Core.Buffer.measure;

let hasLine = (lineNumber, buffer) => {
  let lineIdx = EditorCoreTypes.LineNumber.toZeroBased(lineNumber);
  let lineCount = numberOfLines(buffer);
  lineIdx >= 0 && lineIdx < lineCount;
};
