/*
 * Buffer.re
 *
 * In-memory text buffer representation
 */
module ArrayEx = Utility.ArrayEx;
module OptionEx = Utility.OptionEx;
module Path = Utility.Path;

open EditorCoreTypes;

let tab = Uchar.of_char('\t');

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
  measure: Uchar.t => float,
  saveTick: int,
};

let isEmpty = ({lines, _}) => Array.length(lines) == 0;

module Internal = {
  let createMeasureFunction = (~font, ~indentation) => {
    let indentation = Inferred.value(indentation);

    let Font.{spaceWidth, _} = font;

    uchar =>
      if (Uchar.equal(uchar, tab)) {
        float(IndentationSettings.(indentation.tabSize)) *. spaceWidth;
      } else {
        Font.measure(uchar, font);
      };
  };
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
         | None => fp
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

let ofLines = (~id=0, ~font, rawLines: array(string)) => {
  let indentation = Inferred.implicit(IndentationSettings.default);
  let measure = Internal.createMeasureFunction(~font, ~indentation);

  let lines = rawLines |> Array.map(BufferLine.make(~measure));

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
    indentation,
    syntaxHighlightingEnabled: true,
    lastUsed: 0.,
    font,
    measure,
    saveTick: 0,
  };
};

let measure = (uchar, {measure, _}) => measure(uchar);

let empty = (~font) => ofLines(~font, [||]);

let ofMetadata = (~font, ~id, ~version, ~filePath, ~modified) => {
  ...ofLines(~font, [||]),
  id,
  version,
  filePath,
  modified,
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

let bufferLine = (line, buffer) => {
  let lineIdx = LineNumber.toZeroBased(line);

  if (lineIdx < 0 || lineIdx >= Array.length(buffer.lines)) {
    None;
  } else {
    Some(buffer |> getLine(lineIdx));
  };
};

let rawLine = (line: LineNumber.t, buffer: t) => {
  buffer |> bufferLine(line) |> Option.map(BufferLine.raw);
};

let characterRangeAt = (line, buffer) => {
  buffer
  |> bufferLine(line)
  |> Option.map(bufferLine => {
       let length = BufferLine.lengthSlow(bufferLine);
       let start = CharacterPosition.{line, character: CharacterIndex.zero};

       let stop =
         CharacterPosition.{line, character: CharacterIndex.ofInt(length)};
       CharacterRange.{start, stop};
     });
};

let getNumberOfLines = (buffer: t) => Array.length(buffer.lines);

let tokenAt = (~languageConfiguration, position: CharacterPosition.t, buffer) => {
  let line = position.line;
  let character = position.character;
  let lineNumber = line |> EditorCoreTypes.LineNumber.toZeroBased;
  let numberOfLines = getNumberOfLines(buffer);

  if (lineNumber < 0 || lineNumber >= numberOfLines) {
    None;
  } else {
    let bufferLine = getLine(lineNumber, buffer);
    let f = uchar =>
      LanguageConfiguration.isWordCharacter(uchar, languageConfiguration);
    let startIndex =
      BufferLine.traverse(
        ~f,
        ~direction=`Backwards,
        ~index=character,
        bufferLine,
      )
      |> Option.value(~default=character);
    let stopIndex =
      BufferLine.traverse(
        ~f,
        ~direction=`Forwards,
        ~index=character,
        bufferLine,
      )
      |> Option.value(~default=character);
    Some(
      CharacterRange.{
        start: CharacterPosition.{line, character: startIndex},
        stop: CharacterPosition.{line, character: stopIndex},
      },
    );
  };
};

let lastLine = buffer => {
  buffer.lines |> Array.length |> max(1) |> LineNumber.ofOneBased;
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

let characterRange = (buffer: t) => {
  let start =
    CharacterPosition.{line: LineNumber.zero, character: CharacterIndex.zero};

  let lineCount = getNumberOfLines(buffer);

  if (lineCount > 0) {
    let lastLineCharacters =
      buffer |> getLine(lineCount - 1) |> BufferLine.lengthSlow;

    let stop =
      CharacterPosition.{
        line: LineNumber.ofZeroBased(lineCount - 1),
        character: CharacterIndex.ofInt(lastLineCharacters),
      };

    CharacterRange.{start, stop};
  } else {
    CharacterRange.{start, stop: start};
  };
};

let hasTrailingNewLine = (buffer: t) => {
  let len = Array.length(buffer.lines);
  if (len == 0) {
    false;
  } else {
    let maybeRaw = rawLine(LineNumber.ofZeroBased(len - 1), buffer);
    maybeRaw
    |> Option.map(Utility.StringEx.isWhitespaceOnly)
    |> Option.value(~default=false);
  };
};

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
    (~measure, lines: array(BufferLine.t), update: BufferUpdate.t) => {
  let updateLines = update.lines |> Array.map(BufferLine.make(~measure));
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

  let measure = Internal.createMeasureFunction(~font=buf.font, ~indentation);

  let lines =
    if (originalIndentationValue != newIndentationValue) {
      buf.lines
      |> Array.map(line => {
           let raw = BufferLine.raw(line);
           BufferLine.make(~measure, raw);
         });
    } else {
      buf.lines;
    };
  {...buf, measure, lines, indentation};
};

let getIndentation = buf => buf.indentation |> Inferred.value;

let shouldApplyUpdate = (update: BufferUpdate.t, buf: t) =>
  // First, make sure the version check passes...
  if (update.version > getVersion(buf)) {
    // Then, if this is a full update, do a pass to see if there actually is an update.
    // There are some cases - like undo - where a full update may come through, but actually
    // is just bumping the change tick.
    if (update.isFull) {
      let bufferLines = getLines(buf);

      !ArrayEx.equals(String.equal, bufferLines, update.lines);
    } else {
      true;
    };
  } else {
    false;
  };

let update = (buf: t, update: BufferUpdate.t) =>
  if (shouldApplyUpdate(update, buf)) {
    /***
     If it's a full update, just apply the lines in the entire update
     */
    if (update.isFull) {
      {
        ...buf,
        version: update.version,
        lines:
          update.lines |> Array.map(BufferLine.make(~measure=buf.measure)),
      };
    } else {
      {
        ...buf,
        version: update.version,
        lines: applyUpdate(~measure=buf.measure, buf.lines, update),
      };
    };
  } else {
    buf;
  };

let getFont = buf => buf.font;

let setFont = (font, buf) => {
  let measure =
    Internal.createMeasureFunction(~font, ~indentation=buf.indentation);

  let lines =
    buf.lines
    |> Array.map(line => {
         let raw = BufferLine.raw(line);
         BufferLine.make(~measure, raw);
       });
  {...buf, measure, font, lines};
};

let getSaveTick = ({saveTick, _}) => saveTick;

let incrementSaveTick = buffer => {
  ...buffer,
  modified: false,
  saveTick: buffer.saveTick + 1,
};

let toDebugString = buf => {
  let lines =
    buf
    |> getLines
    |> Array.to_list
    |> List.mapi((idx, str) =>
         "Line  " ++ string_of_int(idx) ++ ": |" ++ str ++ "|"
       )
    |> String.concat("\n");
  Printf.sprintf(
    "Buffer %d (version %d):\n---\n%s\n---\n",
    getId(buf),
    getVersion(buf),
    lines,
  );
};
