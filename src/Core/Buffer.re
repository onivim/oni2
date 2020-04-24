/*
 * Buffer.re
 *
 * In-memory text buffer representation
 */
open EditorCoreTypes;
module ArrayEx = Utility.ArrayEx;
module OptionEx = Utility.OptionEx;
module Path = Utility.Path;

type t = {
  id: int,
  filePath: option(string),
  fileType: option(string),
  lineEndings: option(Vim.lineEnding),
  modified: bool,
  version: int,
  lines: array(BufferLine.t),
  originalUri: option(Uri.t),
  originalLines: option(array(string)),
  indentation: option(IndentationSettings.t),
  syntaxHighlightingEnabled: bool,
  lastUsed: float,
};

let show = _ => "TODO";

let getShortFriendlyName = ({filePath, _}) => {
  Option.map(Filename.basename, filePath);
};

let getMediumFriendlyName =
    (~workingDirectory=?, {filePath: maybeFilePath, _}) => {
  maybeFilePath
  |> Option.map(filePath =>
       switch (BufferPath.parse(filePath)) {
       | Welcome => "Welcome"
       | Version => "Version"
       | Terminal({cmd, _}) => "Terminal - " ++ cmd
       | FilePath(fp) =>
         switch (workingDirectory) {
         | Some(base) => Path.toRelative(~base, fp)
         | _ => Sys.getcwd()
         }
       }
     );
};

let getLineEndings = ({lineEndings, _}) => lineEndings;
let setLineEndings = (lineEndings, buf) => {
  ...buf,
  lineEndings: Some(lineEndings),
};

let getLongFriendlyName = ({filePath: maybeFilePath, _}) => {
  maybeFilePath
  |> Option.map(filePath => {
       switch (BufferPath.parse(filePath)) {
       | Welcome => "Welcome"
       | Version => "Version"
       | Terminal({cmd, _}) => "Terminal - " ++ cmd
       | FilePath(fp) => fp
       }
     });
};

let ofLines = (~id=0, rawLines: array(string)) => {
  let lines =
    rawLines
    |> Array.map(BufferLine.make(~indentation=IndentationSettings.default));

  {
    id,
    version: 0,
    filePath: None,
    fileType: None,
    modified: false,
    lines,
    lineEndings: None,
    originalUri: None,
    originalLines: None,
    indentation: None,
    syntaxHighlightingEnabled: true,
    lastUsed: 0.,
  };
};

let initial = ofLines([||]);

let ofMetadata = (metadata: Vim.BufferMetadata.t) => {
  id: metadata.id,
  version: metadata.version,
  filePath: metadata.filePath,
  lineEndings: None,
  modified: metadata.modified,
  fileType: None,
  lines: [||],
  originalUri: None,
  originalLines: None,
  indentation: None,
  syntaxHighlightingEnabled: true,
  lastUsed: 0.,
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

let getLine = (line: int, buffer: t) => {
  buffer.lines[line];
};

let getLines = (buffer: t) => buffer.lines |> Array.map(BufferLine.raw);

let getOriginalUri = buffer => buffer.originalUri;
let setOriginalUri = (uri, buffer) => {...buffer, originalUri: Some(uri)};

let getOriginalLines = buffer => buffer.originalLines;
let setOriginalLines = (lines, buffer) => {
  ...buffer,
  originalLines: Some(lines),
};

let getVersion = (buffer: t) => buffer.version;
let setVersion = (version: int, buffer: t) => {...buffer, version};

let isModified = (buffer: t) => buffer.modified;
let setModified = (modified: bool, buffer: t) => {...buffer, modified};

let stampLastUsed = buffer => {...buffer, lastUsed: Unix.gettimeofday()};
let getLastUsed = buffer => buffer.lastUsed;

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

let getNumberOfLines = (buffer: t) => Array.length(buffer.lines);

let applyUpdate =
    (~indentation, lines: array(BufferLine.t), update: BufferUpdate.t) => {
  let updateLines = update.lines |> Array.map(BufferLine.make(~indentation));
  let startLine = update.startLine |> Index.toZeroBased;
  let endLine = update.endLine |> Index.toZeroBased;
  ArrayEx.replace(
    ~replacement=updateLines,
    ~start=startLine,
    ~stop=endLine,
    lines,
  );
};

let isIndentationSet = buf => {
  switch (buf.indentation) {
  | Some(_) => true
  | None => false
  };
};
let setIndentation = (indentation, buf) => {
  let lines =
    buf.lines
    |> Array.map(line =>
         BufferLine.raw(line) |> BufferLine.make(~indentation)
       );
  {...buf, lines, indentation: Some(indentation)};
};

let getIndentation = buf => buf.indentation;

let shouldApplyUpdate = (update: BufferUpdate.t, buf: t) => {
  update.version > getVersion(buf);
};

let update = (buf: t, update: BufferUpdate.t) => {
  let indentation =
    Option.value(~default=IndentationSettings.default, buf.indentation);
  if (shouldApplyUpdate(update, buf)) {
    /***
     If it's a full update, just apply the lines in the entire update
     */
    if (update.isFull) {
      {
        ...buf,
        version: update.version,
        lines: update.lines |> Array.map(BufferLine.make(~indentation)),
      };
    } else {
      {
        ...buf,
        version: update.version,
        lines: applyUpdate(~indentation, buf.lines, update),
      };
    };
  } else {
    buf;
  };
};
