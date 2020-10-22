// EditorBuffer is a subset of the buffer model,
// specifically containing the functions and data
// needed by the editor surface.

type t;

let ofBuffer: Oni_Core.Buffer.t => t;
let id: t => int;
let getEstimatedMaxLineLength: t => int;
let numberOfLines: t => int;
let line: (int, t) => Oni_Core.BufferLine.t;
let font: t => Oni_Core.Font.t;
let fileType: t => Oni_Core.Buffer.FileType.t;
let measure: (Uchar.t, t) => float;
