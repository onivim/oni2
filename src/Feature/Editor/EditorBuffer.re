open Oni_Core;

type t = Buffer.t;

let ofBuffer = buffer =>
  if (Buffer.getNumberOfLines(buffer) > 0) {
    buffer;
  } else {
    // #3629 - From the editor UI perspective, never render a totally empty buffer.
    let font = Buffer.getFont(buffer);
    let id = Buffer.getId(buffer);

    Buffer.ofLines(~id, ~font, [|""|]);
  };
let id = Buffer.getId;
let getEstimatedMaxLineLength = Buffer.getEstimatedMaxLineLength;
let numberOfLines = Buffer.getNumberOfLines;
let line = Buffer.getLine;
let font = Buffer.getFont;

let fileType = Buffer.getFileType;
let measure = Buffer.measure;

let hasLine = (lineNumber, buffer) => {
  let lineIdx = EditorCoreTypes.LineNumber.toZeroBased(lineNumber);
  let lineCount = numberOfLines(buffer);
  lineIdx >= 0 && lineIdx < lineCount;
};

let tokenAt = Oni_Core.Buffer.tokenAt;
