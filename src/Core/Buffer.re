/*
 * Buffer.re
 *
 * In-memory text buffer representation
 */
module ArrayEx = Utility.ArrayEx;
module OptionEx = Utility.OptionEx;
module Path = Utility.Path;

open EditorCoreTypes;

module FileType = {
  let default = "plaintext";

  [@deriving show]
  type t =
    | NotSet
    | Inferred(string)
    | Explicit(string);

  let set = (~newFileType: t, current: t) =>
    switch (newFileType, current) {
    | (Explicit(newType), _) => Explicit(newType)
    | (_, Explicit(currentType)) => Explicit(currentType)
    | (Inferred(newType), _) => Inferred(newType)
    | (NotSet, _) => NotSet
    };

  let inferred = fileType => Inferred(fileType);
  let explicit = fileType => Explicit(fileType);
  let none = NotSet;

  let toString =
    fun
    | NotSet => "default"
    | Inferred(fileType)
    | Explicit(fileType) => fileType;

  let ofOption =
    fun
    | None => NotSet
    | Some(fileType) => Inferred(fileType);

  let toOption =
    fun
    | NotSet => None
    | Inferred(fileType)
    | Explicit(fileType) => Some(fileType);
};

type t = {
  id: int,
  filePath: option(string),
  fileType: FileType.t,
  lineEndings: option(Vim.lineEnding),
  modified: bool,
  version: int,
  lines: array(BufferLine.t),
  originalUri: option(Uri.t),
  originalLines: option(array(string)),
  indentation: Inferred.t(IndentationSettings.t),
  syntaxHighlightingEnabled: bool,
  lastUsed: float,
  font: Font.t,
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
       | DebugInput => "Input Debugger"
       | ExtensionDetails => "Extension Details"
       | Welcome => "Welcome"
       | Version => "Version"
       | Terminal({cmd, _}) => "Terminal - " ++ cmd
       | UpdateChangelog => "Updates"
       | Changelog => "Changelog"
       | Image => "Image"
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
       | DebugInput => "Input Debugger"
       | ExtensionDetails => "Extension Details"
       | Welcome => "Welcome"
       | Version => "Version"
       | UpdateChangelog => "Updates"
       | Changelog => "Changelog"
       | Image => "Image"
       | Terminal({cmd, _}) => "Terminal - " ++ cmd
       | FilePath(fp) => fp
       }
     });
};

let ofLines = (~id=0, ~font=Font.default, rawLines: array(string)) => {
  let lines =
    rawLines
    |> Array.map(
         BufferLine.make(~font, ~indentation=IndentationSettings.default),
       );

  {
    id,
    version: 0,
    filePath: None,
    fileType: FileType.NotSet,
    modified: false,
    lines,
    lineEndings: None,
    originalUri: None,
    originalLines: None,
    indentation: Inferred.implicit(IndentationSettings.default),
    syntaxHighlightingEnabled: true,
    lastUsed: 0.,
    font,
  };
};

let initial = ofLines([||]);

let ofMetadata = (~font=Font.default, ~id, ~version, ~filePath, ~modified) => {
  id,
  version,
  filePath,
  lineEndings: None,
  modified,
  fileType: FileType.NotSet,
  lines: [||],
  originalUri: None,
  originalLines: None,
  indentation: Inferred.implicit(IndentationSettings.default),
  syntaxHighlightingEnabled: true,
  lastUsed: 0.,
  font,
};

let getFilePath = (buffer: t) => buffer.filePath;
let setFilePath = (filePath: option(string), buffer) => {
  ...buffer,
  filePath,
};

let getFileType = (buffer: t) => buffer.fileType;
let setFileType = (fileType: FileType.t, buffer: t) => {
  ...buffer,
  fileType: FileType.set(~newFileType=fileType, buffer.fileType),
};

let getId = (buffer: t) => buffer.id;

let getLine = (line: int, buffer: t) => {
  buffer.lines[line];
};

let getLines = (buffer: t) => buffer.lines |> Array.map(BufferLine.raw);

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

// TODO: This method needs a lot of improvements:
// - It's only estimated, as the byte length is quicker to calculate
// - It always traverses the entire buffer - we could be much smarter
//   by using buffer updates and only recalculating subsets.
let getEstimatedMaxLineLength = buffer => {
  let totalLines = getNumberOfLines(buffer);

  let currentMax = ref(0);
  for (idx in 0 to totalLines - 1) {
    let lengthInBytes = buffer |> getLine(idx) |> BufferLine.lengthInBytes;

    if (lengthInBytes > currentMax^) {
      currentMax := lengthInBytes;
    };
  };

  currentMax^;
};

let characterToBytePosition = (position: CharacterPosition.t, buffer) => {
  let line = position.line |> EditorCoreTypes.LineNumber.toZeroBased;

  let bufferLineCount = getNumberOfLines(buffer);

  if (line < bufferLineCount) {
    let bufferLine = getLine(line, buffer);
    let byteIndex =
      BufferLine.getByteFromIndex(~index=position.character, bufferLine);

    Some(
      EditorCoreTypes.(BytePosition.{line: position.line, byte: byteIndex}),
    );
  } else {
    None;
  };
};

let applyUpdate =
    (~indentation, ~font, lines: array(BufferLine.t), update: BufferUpdate.t) => {
  let updateLines =
    update.lines |> Array.map(BufferLine.make(~font, ~indentation));
  let startLine = update.startLine |> EditorCoreTypes.LineNumber.toZeroBased;
  let endLine = update.endLine |> EditorCoreTypes.LineNumber.toZeroBased;
  ArrayEx.replace(
    ~replacement=updateLines,
    ~start=startLine,
    ~stop=endLine,
    lines,
  );
};

let isIndentationSet = buf => {
  buf.indentation |> Inferred.isExplicit;
};

let setIndentation = (indentation, buf) => {
  let originalIndentationValue = buf.indentation |> Inferred.value;
  let indentation = Inferred.update(~new_=indentation, buf.indentation);
  let newIndentationValue = indentation |> Inferred.value;

  let lines =
    if (originalIndentationValue != newIndentationValue) {
      buf.lines
      |> Array.map(line => {
           let raw = BufferLine.raw(line);
           let font = BufferLine.font(line);
           BufferLine.make(~font, ~indentation=newIndentationValue, raw);
         });
    } else {
      buf.lines;
    };
  {...buf, lines, indentation};
};

let getIndentation = buf => buf.indentation |> Inferred.value;

let shouldApplyUpdate = (update: BufferUpdate.t, buf: t) => {
  update.version > getVersion(buf);
};

let update = (buf: t, update: BufferUpdate.t) => {
  let indentation = buf.indentation |> Inferred.value;
  if (shouldApplyUpdate(update, buf)) {
    /***
     If it's a full update, just apply the lines in the entire update
     */
    if (update.isFull) {
      {
        ...buf,
        version: update.version,
        lines:
          update.lines
          |> Array.map(BufferLine.make(~font=buf.font, ~indentation)),
      };
    } else {
      {
        ...buf,
        version: update.version,
        lines: applyUpdate(~indentation, ~font=buf.font, buf.lines, update),
      };
    };
  } else {
    buf;
  };
};

let getFont = buf => buf.font;

let setFont = (font, buf) => {
  let lines =
    buf.lines
    |> Array.map(line => {
         let raw = BufferLine.raw(line);
         let indentation = BufferLine.indentation(line);
         BufferLine.make(~font, ~indentation, raw);
       });
  {...buf, font, lines};
};
