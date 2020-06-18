type t = Oni_Core.Buffer.t;

let ofBuffer = buffer => buffer;
let id = Oni_Core.Buffer.getId;
let getEstimatedMaxLineLength = Oni_Core.Buffer.getEstimatedMaxLineLength;
let numberOfLines = Oni_Core.Buffer.getNumberOfLines;

let line = (idx, buf) => Oni_Core.Buffer.getLine(idx, buf);
