/*
 * Buffer.rei
 *
 * In-memory text buffer representation
 */

open EditorCoreTypes;

type t;

module FileType: {
  [@deriving show]
  type t;

  let default: string;

  let none: t;
  let inferred: string => t;
  let explicit: string => t;

  let ofOption: option(string) => t;

  let toString: t => string;
  let toOption: t => option(string);
};

let empty: (~font: Font.t) => t;

let show: t => string;

let ofLines: (~id: int=?, ~font: Font.t, array(string)) => t;
let ofMetadata:
  (
    ~font: Font.t,
    ~id: int,
    ~version: int,
    ~filePath: option(string),
    ~modified: bool
  ) =>
  t;

let characterToBytePosition:
  (CharacterPosition.t, t) => option(BytePosition.t);

let getId: t => int;
let getUri: t => Uri.t;
let getFilePath: t => option(string);
let setFilePath: (option(string), t) => t;

let getEstimatedMaxLineLength: t => int;

let measure: (Uchar.t, t) => float;

let getLineEndings: t => option(Vim.lineEnding);
let setLineEndings: (Vim.lineEnding, t) => t;

let getShortFriendlyName: t => option(string);
let getMediumFriendlyName: (~workingDirectory: string=?, t) => option(string);
let getLongFriendlyName: t => option(string);

let getFileType: t => FileType.t;
let setFileType: (FileType.t, t) => t;
let getLine: (int, t) => BufferLine.t;
let getLines: t => array(string);
let getNumberOfLines: t => int;

let getVersion: t => int;
let setVersion: (int, t) => t;

let isModified: t => bool;
let setModified: (bool, t) => t;

let isIndentationSet: t => bool;
let setIndentation: (Inferred.t(IndentationSettings.t), t) => t;
let getIndentation: t => IndentationSettings.t;

let isSyntaxHighlightingEnabled: t => bool;
let disableSyntaxHighlighting: t => t;

let stampLastUsed: t => t;
let getLastUsed: t => float;

let shouldApplyUpdate: (BufferUpdate.t, t) => bool;
let update: (t, BufferUpdate.t) => t;

let getFont: t => Font.t;
let setFont: (Font.t, t) => t;

let getSaveTick: t => int;
let incrementSaveTick: t => t;

let toDebugString: t => string;
