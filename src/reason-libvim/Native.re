type buffer = Types.buffer;
type lineEnding = Types.lineEnding;

external vimInit: unit => unit = "libvim_vimInit";
external vimInput: string => unit = "libvim_vimInput";
external vimCommand: string => unit = "libvim_vimCommand";

external vimGetMode: unit => Types.mode = "libvim_vimGetMode";

external vimBufferOpen: string => buffer = "libvim_vimBufferOpen";
external vimBufferGetId: buffer => int = "libvim_vimBufferGetId";
external vimBufferGetById: int => option(buffer) = "libvim_vimBufferGetById";
external vimBufferGetCurrent: unit => buffer = "libvim_vimBufferGetCurrent";
external vimBufferGetFilename: buffer => option(string) =
  "libvim_vimBufferGetFilename";
external vimBufferGetFiletype: buffer => option(string) =
  "libvim_vimBufferGetFiletype";

external vimBufferGetFileFormat: buffer => option(lineEnding) =
  "libvim_vimBufferGetFileFormat";
external vimBufferSetFileFormat: (buffer, lineEnding) => unit =
  "libvim_vimBufferSetFileFormat";
external vimBufferGetLine: (buffer, int) => string = "libvim_vimBufferGetLine";
external vimBufferGetLineCount: buffer => int = "libvim_vimBufferGetLineCount";
external vimBufferGetModified: buffer => bool = "libvim_vimBufferGetModified";
external vimBufferGetChangedTick: buffer => int =
  "libvim_vimBufferGetChangedTick";
external vimBufferSetCurrent: buffer => unit = "libvim_vimBufferSetCurrent";
external vimBufferSetLines: (buffer, int, int, array(string)) => unit =
  "libvim_vimBufferSetLines";

external vimBufferIsReadOnly: buffer => bool = "libvim_vimBufferGetReadOnly";
external vimBufferIsModifiable: buffer => bool =
  "libvim_vimBufferGetModifiable";
external vimBufferSetReadOnly: (bool, buffer) => unit =
  "libvim_vimBufferSetReadOnly";
external vimBufferSetModifiable: (bool, buffer) => unit =
  "libvim_vimBufferSetModifiable";

external vimCommandLineGetCompletions: unit => array(string) =
  "libvim_vimCommandLineGetCompletions";
external vimCommandLineGetPosition: unit => int =
  "libvim_vimCommandLineGetPosition";
external vimCommandLineGetText: unit => option(string) =
  "libvim_vimCommandLineGetText";
external vimCommandLineGetType: unit => Types.cmdlineType =
  "libvim_vimCommandLineGetType";

external vimCursorGetLine: unit => int = "libvim_vimCursorGetLine";
external vimCursorGetColumn: unit => int = "libvim_vimCursorGetColumn";
external vimCursorSetPosition: (int, int) => unit =
  "libvim_vimCursorSetPosition";

external vimOptionSetTabSize: int => unit = "libvim_vimOptionSetTabSize";
external vimOptionSetInsertSpaces: bool => unit =
  "libvim_vimOptionSetInsertSpaces";
external vimOptionSetLineComment: string => unit =
  "libvim_vimOptionSetLineComment";
external vimOptionGetInsertSpaces: unit => bool =
  "libvim_vimOptionGetInsertSpaces";
external vimOptionGetTabSize: unit => int = "libvim_vimOptionGetTabSize";

external vimSearchGetMatchingPair: unit => option((int, int)) =
  "libvim_vimSearchGetMatchingPair";

external vimSearchGetHighlights: (int, int) => array((int, int, int, int)) =
  "libvim_vimSearchGetHighlights";

external vimUndoSaveCursor: unit => unit = "libvim_vimUndoSaveCursor";
external vimUndoSaveRegion: (int, int) => unit = "libvim_vimUndoSaveRegion";
external vimUndoSync: int => unit = "libvim_vimUndoSync";

external vimVisualGetRange: unit => (int, int, int, int) =
  "libvim_vimVisualGetRange";

external vimVisualGetType: unit => Types.visualType =
  "libvim_vimVisualGetType";

external vimWindowGetWidth: unit => int = "libvim_vimWindowGetWidth";
external vimWindowGetHeight: unit => int = "libvim_vimWindowGetHeight";
external vimWindowGetLeftColumn: unit => int = "libvim_vimWindowGetLeftColumn";
external vimWindowGetTopLine: unit => int = "libvim_vimWindowGetTopLine";

external vimWindowSetHeight: int => unit = "libvim_vimWindowSetHeight";
external vimWindowSetTopLeft: (int, int) => unit =
  "libvim_vimWindowSetTopLeft";
external vimWindowSetWidth: int => unit = "libvim_vimWindowSetWidth";

type bufferUpdateCallback = (buffer, int, int, int) => unit;
/* external vimSetBufferUpdateCallback: bufferUpdateCallback => unit; */
