open EditorCoreTypes;

type t = Native.buffer;

let openFile = (filePath: string) => {
  let ret = Native.vimBufferOpen(filePath);

  BufferInternal.checkCurrentBufferForUpdate();
  ret;
};

let getFilename = (buffer: t) => {
  Native.vimBufferGetFilename(buffer);
};

let isReadOnly = Native.vimBufferIsReadOnly;
let setReadOnly = (~readOnly) => Native.vimBufferSetReadOnly(readOnly);
let isModifiable = Native.vimBufferIsModifiable;
let setModifiable = (~modifiable) =>
  Native.vimBufferSetModifiable(modifiable);

let getFiletype = (buffer: t) => {
  Native.vimBufferGetFiletype(buffer);
};

let getVersion = (buffer: t) => {
  Native.vimBufferGetChangedTick(buffer);
};

let isModified = (buffer: t) => {
  Native.vimBufferGetModified(buffer);
};

let getLineCount = (buffer: t) => {
  Native.vimBufferGetLineCount(buffer);
};

let getLine = (buffer: t, line: Index.t) => {
  Native.vimBufferGetLine(buffer, Index.toOneBased(line));
};

let getId = (buffer: t) => {
  Native.vimBufferGetId(buffer);
};

let getById = (id: int) => {
  Native.vimBufferGetById(id);
};

let getCurrent = () => {
  Native.vimBufferGetCurrent();
};

let setCurrent = (buffer: t) => {
  Native.vimBufferSetCurrent(buffer);
  BufferInternal.checkCurrentBufferForUpdate();
};

let getLineEndings = (buffer: t) => {
  Native.vimBufferGetFileFormat(buffer);
};

let setLineEndings = (buffer, lineEnding) => {
  Native.vimBufferSetFileFormat(buffer, lineEnding);
  let newLineEndings = getLineEndings(buffer);
  let id = getId(buffer);
  newLineEndings
  |> Option.iter(lineEndings =>
       Event.dispatch2(id, lineEndings, Listeners.bufferLineEndingsChanged)
     );
};

let setLines = (~start=?, ~stop=?, ~lines, buffer) => {
  let startLine =
    switch (start) {
    | Some(v) => Index.toOneBased(v) - 1
    | None => 0
    };

  let endLine =
    switch (stop) {
    | Some(v) => Index.toOneBased(v) - 1
    | None => (-1)
    };

  Native.vimBufferSetLines(buffer, startLine, endLine, lines);
};

let onEnter = (f: Listeners.bufferListener) => {
  Event.add(f, Listeners.bufferEnter);
};

let onModifiedChanged = (f: Listeners.bufferModifiedChangedListener) => {
  Event.add2(f, Listeners.bufferModifiedChanged);
};

let onUpdate = (f: Listeners.bufferUpdateListener) => {
  Event.add(f, Listeners.bufferUpdate);
};

let onFilenameChanged = (f: Listeners.bufferMetadataChangedListener) => {
  Event.add(f, Listeners.bufferFilenameChanged);
};

let onFiletypeChanged = (f: Listeners.bufferMetadataChangedListener) => {
  Event.add(f, Listeners.bufferFiletypeChanged);
};

let onLineEndingsChanged = (f: (int, Oni_Core.LineEnding.t) => unit) => {
  Event.add2(f, Listeners.bufferLineEndingsChanged);
};

let onWrite = (f: Listeners.bufferWriteListener) => {
  Event.add(f, Listeners.bufferWrite);
};
