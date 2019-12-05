/*
 * Buffer.re
 *
 * In-memory text buffer representation
 */

open Types;

type t = {
  id: int,
  filePath: option(string),
  fileType: option(string),
  modified: bool,
  version: int,
  lines: array(string),
  indentation: option(IndentationSettings.t),
  syntaxHighlightingEnabled: bool,
};

let show = _ => "TODO";

let ofLines = (lines: array(string)) => {
  id: 0,
  version: 0,
  filePath: None,
  fileType: None,
  modified: false,
  lines,
  indentation: None,
  syntaxHighlightingEnabled: true,
};

let empty = ofLines([||]);

let ofMetadata = (metadata: Vim.BufferMetadata.t) => {
  id: metadata.id,
  version: metadata.version,
  filePath: metadata.filePath,
  modified: metadata.modified,
  fileType: None,
  lines: [||],
  indentation: None,
  syntaxHighlightingEnabled: true,
};

let getFilePath = (buffer: t) => buffer.filePath;

let setFilePath = (filePath: option(string), buffer) => {
  ...buffer,
  filePath,
};

let getFileType = (buffer: t) => buffer.fileType;

let setFileType = (fileType: option(string), buffer: t) => {
  ...buffer,
  fileType,
};

let getId = (buffer: t) => buffer.id;

let getLine = (buffer: t, line: int) => buffer.lines[line];

let getLines = (buffer: t) => buffer.lines;

let getVersion = (buffer: t) => buffer.version;

let setVersion = (version: int, buffer: t) => {...buffer, version};

let isModified = (buffer: t) => buffer.modified;

let setModified = (modified: bool, buffer: t) => {...buffer, modified};

let isSyntaxHighlightingEnabled = (buffer: t) =>
  buffer.syntaxHighlightingEnabled;
let disableSyntaxHighlighting = (buffer: t) => {
  ...buffer,
  syntaxHighlightingEnabled: false,
};

let getUri = (buffer: t) => {
  switch (buffer.filePath) {
  | None => Uri.fromMemory(string_of_int(buffer.id))
  | Some(v) => Uri.fromPath(v)
  };
};

/*
 * TODO:
 * - Handle variable tab sizes, based on indentation settings
 * - Handle multibyte characters
 */
let getLineLength = (buffer: t, line: int) => {
  let line = getLine(buffer, line);
  String.length(line);
};

let getNumberOfLines = (buffer: t) => Array.length(buffer.lines);

let slice = (~lines: array(string), ~start, ~length, ()) => {
  let len = Array.length(lines);
  if (start >= len) {
    [||];
  } else {
    let start = max(start, 0);
    let len = min(start + length, len) - start;
    if (len <= 0) {
      [||];
    } else {
      Array.sub(lines, start, len);
    };
  };
};

let applyUpdate = (lines: array(string), update: BufferUpdate.t) => {
  let updateLines = update.lines;
  let startLine = update.startLine |> Index.toZeroBasedInt;
  let endLine = update.endLine |> Index.toZeroBasedInt;
  if (Array.length(lines) == 0) {
    updateLines;
  } else if (startLine >= Array.length(lines)) {
    let ret = Array.concat([lines, updateLines]);
    ret;
  } else {
    let prev = slice(~lines, ~start=0, ~length=startLine, ());
    let post =
      slice(
        ~lines,
        ~start=endLine,
        ~length=Array.length(lines) - endLine,
        (),
      );

    let lines = update.lines;

    Array.concat([prev, lines, post]);
  };
};

let isIndentationSet = buf => {
  switch (buf.indentation) {
  | Some(_) => true
  | None => false
  };
};
let setIndentation = (indent, buf) => {...buf, indentation: Some(indent)};

let getIndentation = buf => buf.indentation;

let shouldApplyUpdate = (update: BufferUpdate.t, buf: t) => {
  update.version > getVersion(buf);
};

let update = (buf: t, update: BufferUpdate.t) =>
  if (shouldApplyUpdate(update, buf)) {
    /***
     If it's a full update, just apply the lines in their entiretupdate.endLiney
     */
    if (update.isFull) {
      {...buf, version: update.version, lines: update.lines};
    } else {
      {
        ...buf,
        version: update.version,
        lines: applyUpdate(buf.lines, update),
      };
    };
  } else {
    buf;
  };
