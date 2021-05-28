open EditorCoreTypes;
open Oni_Core;
// EditorBuffer is a subset of the buffer model,
// specifically containing the functions and data
// needed by the editor surface.

type t;

let ofBuffer: Buffer.t => t;
let id: t => int;
let getEstimatedMaxLineLength: t => int;
let numberOfLines: t => int;
let line: (int, t) => BufferLine.t;
let hasLine: (EditorCoreTypes.LineNumber.t, t) => bool;
let font: t => Font.t;
let fileType: t => Buffer.FileType.t;
let measure: (Uchar.t, t) => float;

let tokenAt:
  (~languageConfiguration: LanguageConfiguration.t, CharacterPosition.t, t) =>
  option(CharacterRange.t);
