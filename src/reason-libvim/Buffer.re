open EditorCoreTypes;

type t = Native.buffer;

let openFile = (filePath: string) => {
  let ret = Native.vimBufferOpen(filePath);

  BufferInternal.checkCurrentBufferForUpdate();
  ret;
};

let loadFile = (filePath: string) => {
  Native.vimBufferLoad(filePath);
};

let make = Native.vimBufferNew;

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

let getLine = (buffer: t, line: LineNumber.t) => {
  Native.vimBufferGetLine(buffer, LineNumber.toOneBased(line));
};

let getLines = (buffer: t) => {
  let count = getLineCount(buffer);

  Array.init(count, idx => {Native.vimBufferGetLine(buffer, idx + 1)});
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

let setLines =
    (~undoable=false, ~start=?, ~stop=?, ~shouldAdjustCursors, ~lines, buffer) => {
  BufferUpdateTracker.watch(
    ~shouldAdjustCursors,
    () => {
      let startLine =
        switch (start) {
        | Some(v) => LineNumber.toOneBased(v) - 1
        | None => 0
        };

      let endLine =
        switch (stop) {
        | Some(v) => LineNumber.toOneBased(v) - 1
        | None => (-1)
        };

      if (undoable) {
        let undoEndLine = endLine == (-1) ? getLineCount(buffer) : endLine;
        Undo.saveRegion(startLine - 1, undoEndLine + 1);
      };

      Native.vimBufferSetLines(buffer, startLine, endLine, lines);
    },
  );
};

let applyEdits = (~shouldAdjustCursors, ~edits, buffer) => {
  let provider = idx => {
    let lineCount = getLineCount(buffer);
    if (idx >= lineCount) {
      None;
    } else {
      Some(getLine(buffer, LineNumber.ofZeroBased(idx)));
    };
  };

  let previousBuffer = getCurrent();
  let previousBufferId = getId(previousBuffer);
  let bufferId = getId(buffer);

  // Swap current buffer temporarily
  if (bufferId != previousBufferId) {
    setCurrent(buffer);
  };

  let prevModified = isModified(buffer);

  Undo.sync(~force=true);

  // Sort edits prior to applying, such that last edits
  // are applied first.
  let edits = Edit.sort(edits);

  let rec loop = edits => {
    switch (edits) {
    | [] => Ok()
    | [hd, ...tail] =>
      let result = Edit.applyEdit(~provider, hd);
      switch (result) {
      | Ok({oldStartLine, oldEndLine, newLines}) =>
        EditorCoreTypes.
          // Save previous lines for undo
          (
            {
              Undo.saveRegion(
                oldStartLine |> LineNumber.toZeroBased,
                (oldEndLine |> LineNumber.toZeroBased) + 2,
              );

              let lineCount = getLineCount(buffer);
              let stop =
                if (oldEndLine |> LineNumber.toZeroBased >= lineCount) {
                  None;
                } else {
                  Some(LineNumber.(oldEndLine + 1));
                };

              setLines(
                ~shouldAdjustCursors,
                ~start=oldStartLine,
                ~stop?,
                ~lines=newLines,
                buffer,
              );
              loop(tail);
            }
          )
      | Error(_) as err => err
      };
    };
  };

  let ret = loop(edits);

  let newModified = isModified(buffer);

  if (prevModified != newModified) {
    Event.dispatch2(bufferId, newModified, Listeners.bufferModifiedChanged);
  };

  if (previousBufferId != bufferId) {
    setCurrent(previousBuffer);
  };
  ret;
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

let onLineEndingsChanged = (f: (int, Types.lineEnding) => unit) => {
  Event.add2(f, Listeners.bufferLineEndingsChanged);
};

let onWrite = (f: Listeners.bufferWriteListener) => {
  Event.add(f, Listeners.bufferWrite);
};
