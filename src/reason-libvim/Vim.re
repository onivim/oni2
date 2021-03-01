open EditorCoreTypes;

type lineEnding = Types.lineEnding;

module AutoClosingPairs = AutoClosingPairs;
module AutoCommands = AutoCommands;
module AutoIndent = AutoIndent;
module Buffer = Buffer;
module BufferMetadata = BufferMetadata;
module BufferUpdate = BufferUpdate;
module Clear = Clear;
module Clipboard = Clipboard;
module ColorScheme = ColorScheme;
module CommandLine = CommandLine;
module Context = Context;
module Cursor = Cursor;
module Edit = Edit;
module Effect = Effect;
module Event = Event;
module Format = Format;
module Functions = Functions;
module Goto = Goto;
module Mapping = Mapping;
module Operator = Operator;
module Scroll = Scroll;
module Split = Split;
module TabPage = TabPage;
module Mode = Mode;
module SubMode = SubMode;
module Options = Options;
module Search = Search;
module Setting = Setting;
module Types = Types;
module Testing = {
  module Undo = Undo;
};
module Visual = Visual;
module VisualRange = VisualRange;
module Window = Window;
module ViewLineMotion = ViewLineMotion;
module Yank = Yank;

/**
[replace(~insert, ~start, ~len, str)] replaces the string segment
from [start] to [len] with str
*/
let replace = (~insert, ~start, ~len, str) => {
  let totalLen = String.length(str);
  let prefix = String.sub(str, 0, start);
  let postfix = String.sub(str, start + len + 1, totalLen - len - start - 1);
  prefix ++ insert ++ postfix;
};

module Internal = {
  let nativeFormatRequestToEffect: Native.formatRequest => Format.effect =
    ({bufferId, startLine, endLine, returnCursor, formatType, lineCount}) => {
      let adjustCursor = returnCursor == 0 ? false : true;
      let formatType =
        switch (formatType) {
        | Native.Indentation => Format.Indentation
        | Native.Formatting => Format.Formatting
        };

      if (startLine <= 1 && endLine >= lineCount) {
        Format.Buffer({formatType, bufferId, adjustCursor});
      } else {
        Format.Range({
          formatType,
          bufferId,
          adjustCursor,
          startLine: EditorCoreTypes.LineNumber.ofOneBased(startLine),
          endLine: EditorCoreTypes.LineNumber.ofOneBased(endLine),
        });
      };
    };

  let getDefaultCursors = (cursors: list(BytePosition.t)) =>
    if (cursors == []) {
      [Cursor.get()];
    } else {
      cursors;
    };

  let getPrecedingWhitespace = (~max: int, str) => {
    let len = String.length(str);

    let rec loop = idx =>
      if (idx >= max || idx >= len) {
        idx;
      } else {
        let c = str.[idx];
        if (c == ' ' || c == '\t') {
          loop(idx + 1);
        } else {
          idx;
        };
      };

    let lastWhitespaceIndex = loop(0);

    String.sub(str, 0, lastWhitespaceIndex);
  };

  let isEmpty = (~max: int, str) => {
    let len = String.length(str);

    let rec loop = idx =>
      if (idx >= max) {
        false;
      } else if (idx >= len) {
        true;
      } else {
        let c = str.[idx];
        if (c == ' ' || c == '\t') {
          loop(idx + 1);
        } else {
          false;
        };
      };

    loop(0);
  };

  let getPrecedingWhitespaceCount = (~max: int, str) => {
    let len = String.length(str);

    let rec loop = (idx, count) =>
      if (idx >= max || idx >= len) {
        count;
      } else {
        let c = str.[idx];
        if (c == ' ' || c == '\t') {
          loop(idx + 1, count + 1);
        } else {
          count;
        };
      };

    loop(0, 0);
  };
};

let queue = f =>
  GlobalState.queuedFunctions := [f, ...GlobalState.queuedFunctions^];

let flushQueue = () => {
  GlobalState.queuedFunctions^ |> List.rev |> List.iter(f => f());

  GlobalState.queuedFunctions := [];
};

let queueEffect = eff => {
  queue(() => {
    Event.dispatch(eff, Listeners.effect);

    GlobalState.effects := [eff, ...GlobalState.effects^];
  });
};

let runWith = (~context: Context.t, f) => {
  let currentBufferId = Buffer.getCurrent() |> Buffer.getId;

  if (currentBufferId != context.bufferId) {
    let currentBuffer = Buffer.getById(context.bufferId);

    // TODO: Turn to result?
    currentBuffer |> Option.iter(Buffer.setCurrent);
  };

  if (Window.getWidth() != context.width) {
    Window.setWidth(context.width);
  };

  if (Window.getHeight() != context.height) {
    Window.setHeight(context.height);
  };

  if (Window.getTopLine() != context.topLine
      || Window.getLeftColumn() != context.leftColumn) {
    Window.setTopLeft(context.topLine, context.leftColumn);
  };

  Options.setTabSize(context.tabSize);
  Options.setInsertSpaces(context.insertSpaces);

  let prevSearchString = Search.getSearchPattern();
  let oldBuf = Buffer.getCurrent();
  let prevMode = Mode.trySet(context.mode);
  let prevModified = Buffer.isModified(oldBuf);
  let prevLineEndings = Buffer.getLineEndings(oldBuf);

  GlobalState.context := Some(context);
  GlobalState.viewLineMotion := Some(context.viewLineMotion);
  GlobalState.screenPositionMotion := Some(context.screenCursorMotion);
  GlobalState.effects := [];
  GlobalState.toggleComments := Some(context.toggleComments);
  GlobalState.additionalCursors := [];

  let mode = f();

  GlobalState.context := None;
  GlobalState.viewLineMotion := None;
  GlobalState.screenPositionMotion := None;
  GlobalState.toggleComments := None;

  let newBuf = Buffer.getCurrent();
  //let newMode = Mode.current();
  let newModified = Buffer.isModified(newBuf);
  let newLineEndings = Buffer.getLineEndings(newBuf);

  let newSearchString = Search.getSearchPattern();

  if (newSearchString != prevSearchString) {
    queueEffect(SearchStringChanged(newSearchString));
  };

  // Apply additional cursors
  let mode =
    switch (mode) {
    | Mode.Insert({cursors}) =>
      Mode.Insert({cursors: cursors @ GlobalState.additionalCursors^})
    | mode => mode
    };

  BufferInternal.checkCurrentBufferForUpdate();

  if (!Mode.isCommandLine(mode) || !Mode.isCommandLine(prevMode)) {
    if (Mode.isCommandLine(mode)) {
      Event.dispatch(
        CommandLineInternal.getState(),
        Listeners.commandLineEnter,
      );
    } else if (Mode.isCommandLine(prevMode)) {
      Event.dispatch((), Listeners.commandLineLeave);
    };
  } else if (Mode.isCommandLine(mode)) {
    Event.dispatch(
      CommandLineInternal.getState(),
      Listeners.commandLineUpdate,
    );
  };

  let id = Buffer.getId(newBuf);
  if (prevModified != newModified) {
    Event.dispatch2(id, newModified, Listeners.bufferModifiedChanged);
  };

  if (newLineEndings != prevLineEndings) {
    newLineEndings
    |> Option.iter(lineEndings =>
         Event.dispatch2(id, lineEndings, Listeners.bufferLineEndingsChanged)
       );
  };

  flushQueue();
  (
    {...Context.current(), mode, autoClosingPairs: context.autoClosingPairs},
    GlobalState.effects^ |> List.rev,
  );
};

let _onAutocommand = (autoCommand: Types.autocmd, buffer: Buffer.t) => {
  Event.dispatch2(autoCommand, buffer, Listeners.autocmd);

  switch (autoCommand) {
  | BufWritePost
  | FileWritePost =>
    let id = Buffer.getId(buffer);
    queue(() => Event.dispatch(id, Listeners.bufferWrite));
  | _ => ()
  };
};

let _onBufferChanged =
    (buffer: Buffer.t, startLine: int, endLine: int, extra: int) => {
  let update =
    BufferUpdate.createIncremental(~buffer, ~startLine, ~endLine, ~extra);

  BufferInternal.notifyUpdate(buffer);

  // TODO: Fix this up
  Event.dispatch(update, Listeners.bufferUpdate);
};

let _onDirectoryChanged = _ => {
  queue(() => Event.dispatch(Sys.getcwd(), Listeners.directoryChanged));
};

let _onIntro = () => {
  queue(() => Event.dispatch((), Listeners.intro));
};

let _onMessage = (priority, title, message) => {
  switch (GlobalState.overriddenMessageHandler^) {
  | None =>
    queue(() => Event.dispatch3(priority, title, message, Listeners.message))
  | Some(handler) => handler(priority, title, message)
  };
};

let _onQuit = (q, f) => {
  queue(() => Event.dispatch2(q, f, Listeners.quit));
};

let _onWindowMovement = (mt, c) => {
  queue(() => Event.dispatch2(mt, c, Listeners.windowMovement));
};

let _onWindowSplit = (st, p) => {
  queueEffect(Effect.WindowSplit(Split.ofNative(st, p)));
};

let _onYank =
    (
      lines,
      yankTypeInt,
      operator,
      register,
      startLine,
      startColumn,
      endLine,
      endColumn,
    ) => {
  queue(() =>
    Event.dispatch(
      Yank.create(
        ~lines,
        ~yankTypeInt,
        ~operator,
        ~register,
        ~startLine,
        ~startColumn,
        ~endLine,
        ~endColumn,
        (),
      ),
      Listeners.yank,
    )
  );
};

let _onWriteFailure = (reason, buffer) => {
  queue(() => Event.dispatch2(reason, buffer, Listeners.writeFailure));
};

let _onStopSearch = () => {
  queueEffect(Effect.SearchClearHighlights);
};

let _onUnhandledEscape = () => {
  queue(() => Event.dispatch((), Listeners.unhandledEscape));
};

let _clipboardGet = (regname: int) => {
  switch (Clipboard._provider^) {
  | None => None
  | Some(v) => v(regname)
  };
};

let _onFormat = formatRequest => {
  queue(() =>
    Event.dispatch(
      Effect.Format(formatRequest |> Internal.nativeFormatRequestToEffect),
      Listeners.effect,
    )
  );
};

let _onAutoIndent = (lnum: int, sourceLine: string) => {
  let buf = Buffer.getCurrent();
  let lineCount = Buffer.getLineCount(buf);

  // lnum is one-based, coming from Vim - we'd only have a beforePreviousLine if the current line is line 3
  let beforePreviousLine =
    if (lnum >= 3 && lnum <= lineCount) {
      lnum - 2 |> LineNumber.ofOneBased |> Buffer.getLine(buf) |> Option.some;
    } else {
      None;
    };

  let beforeLine =
    if (lnum >= 2 && lnum <= lineCount) {
      lnum - 1 |> LineNumber.ofOneBased |> Buffer.getLine(buf);
    } else {
      ""; // This should never happen... but follow the Vim convention for empty lines.
    };

  let indentAction =
    GlobalState.context^
    |> Option.map(({autoIndent, _}: Context.t) => autoIndent)
    |> Option.map(fn => fn(~previousLine=beforeLine, ~beforePreviousLine))
    |> Option.value(~default=AutoIndent.KeepIndent);

  // A note about [sourceLine]:
  // This line could be the previous line (in the case of <CR> or `o`),
  // or it could be the line _after_ the current line (in the case of `O`).
  // The current indentation comes from the source line, which causes some
  // challenge, as the auto-indent rules are based on the previous two lines -
  let aboveWhitespace =
    Internal.getPrecedingWhitespaceCount(~max=100, beforeLine);
  let afterWhitespace =
    Internal.getPrecedingWhitespaceCount(~max=100, sourceLine);

  let isPreviousLineEmpty = Internal.isEmpty(~max=100, beforeLine);

  // The [indentOffset] is computed to offset the difference between the previous line and source line,
  // to normalize the indentation provided by the callback function.
  let indentOffset =
    if (!isPreviousLineEmpty && aboveWhitespace > afterWhitespace) {
      1;
    } else if (!isPreviousLineEmpty && aboveWhitespace < afterWhitespace) {
      (-1);
    } else {
      0;
    };

  switch (indentAction) {
  | AutoIndent.IncreaseIndent => 1 + indentOffset
  | AutoIndent.KeepIndent => 0 + indentOffset
  | AutoIndent.DecreaseIndent => (-1) + indentOffset
  };
};

let _onCursorMoveScreenLine =
    (motion: ViewLineMotion.t, count: int, line: int) => {
  let startLine = LineNumber.ofOneBased(line);
  GlobalState.viewLineMotion^
  |> Option.map(f => f(~motion, ~count, ~startLine))
  |> Option.map(LineNumber.toOneBased)
  |> Option.value(~default=line);
};

let _onCursorMoveScreenPosition =
    (direction, count: int, line: int, byte: int, wantByte: int) => {
  let startLine = LineNumber.ofOneBased(line);
  let startByte = ByteIndex.ofInt(byte);
  GlobalState.screenPositionMotion^
  |> Option.map(f =>
       f(
         ~direction,
         ~count,
         ~line=startLine,
         ~currentByte=startByte,
         ~wantByte=ByteIndex.ofInt(wantByte),
       )
     )
  |> Option.map((bytePosition: BytePosition.t) => {
       (
         bytePosition.line |> LineNumber.toOneBased,
         bytePosition.byte |> ByteIndex.toInt,
       )
     })
  |> Option.value(~default=(line, wantByte));
};

let _onGoto = (_line: int, _column: int, gotoType: Goto.effect) => {
  queueEffect(Effect.Goto(gotoType));
};

let _onOutput = (cmd, output) => {
  queueEffect(Effect.Output({cmd, output}));
};

let _onClear = (target: Clear.target, count: int) => {
  queueEffect(Effect.Clear(Clear.{target, count}));
};

let _onTabPage = (msg: TabPage.effect) => {
  queue(() => Event.dispatch(Effect.TabPage(msg), Listeners.effect));
};

let _onTerminal = terminalRequest => {
  queue(() => Event.dispatch(terminalRequest, Listeners.terminalRequested));
};

let _onVersion = () => {
  queue(() => Event.dispatch((), Listeners.version));
};

let _onSettingChanged = (setting: Setting.t) => {
  queue(() =>
    Event.dispatch(Effect.SettingChanged(setting), Listeners.effect)
  );
};

let _onScroll = (direction: Scroll.direction, count: int) => {
  queueEffect(Effect.Scroll({direction, count}));
};

let _onColorSchemeChanged = (maybeScheme: option(string)) => {
  queue(() => {
    Event.dispatch(Effect.ColorSchemeChanged(maybeScheme), Listeners.effect)
  });
};

let _colorSchemesGet = pattern => {
  GlobalState.context^
  |> Option.map(({colorSchemeProvider, _}: Context.t) =>
       colorSchemeProvider(pattern)
     )
  |> Option.value(~default=[||]);
};

let _onMacroStartRecording = (register: char) => {
  queue(() => {
    Event.dispatch(
      Effect.MacroRecordingStarted({register: register}),
      Listeners.effect,
    )
  });
};

let _onMacroStopRecording = (register: char, value: option(string)) => {
  queue(() => {
    Event.dispatch(
      Effect.MacroRecordingStopped({register, value}),
      Listeners.effect,
    )
  });
};

let _onInputMap = (mapping: Mapping.t) => {
  queueEffect(Map(mapping));
};

let _onInputUnmap = (mode: Mapping.mode, keys: option(string)) => {
  queueEffect(Unmap({mode, keys}));
};

let _onToggleComments = (buf: Buffer.t, startLine: int, endLine: int) => {
  let count = endLine - startLine + 1;
  let currentLines =
    Array.init(count, i => {
      Buffer.getLine(buf, LineNumber.ofOneBased(startLine + i))
    });

  GlobalState.toggleComments^
  |> Option.map(f => f(currentLines))
  |> Option.value(~default=currentLines);
};

let _onCursorAdd = (oneBasedLine: int, column: int) => {
  GlobalState.additionalCursors :=
    [
      BytePosition.{
        line: LineNumber.ofOneBased(oneBasedLine),
        byte: ByteIndex.ofInt(column),
      },
      ...GlobalState.additionalCursors^,
    ];
};

let _onGetChar = mode => {
  let mode' =
    switch (mode) {
    | 0 => Functions.GetChar.Immediate
    | 1 => Functions.GetChar.Peek
    | _ => Functions.GetChar.Wait
    };

  let c =
    GlobalState.context^
    |> Option.map(({functionGetChar, _}: Context.t) =>
         functionGetChar(mode')
       )
    |> Option.value(~default=char_of_int(0));
  (int_of_char(c), 0);
};

let init = () => {
  Callback.register("lv_clipboardGet", _clipboardGet);
  Callback.register("lv_onBufferChanged", _onBufferChanged);
  Callback.register("lv_onAutocommand", _onAutocommand);
  Callback.register("lv_onAutoIndent", _onAutoIndent);
  Callback.register("lv_onClear", _onClear);
  Callback.register("lv_getColorSchemesCallback", _colorSchemesGet);
  Callback.register("lv_onColorSchemeChanged", _onColorSchemeChanged);
  Callback.register("lv_onDirectoryChanged", _onDirectoryChanged);
  Callback.register("lv_onFormat", _onFormat);
  Callback.register("lv_onGoto", _onGoto);
  Callback.register("lv_onTabPage", _onTabPage);
  Callback.register("lv_onIntro", _onIntro);
  Callback.register("lv_onMessage", _onMessage);
  Callback.register("lv_onMacroStartRecording", _onMacroStartRecording);
  Callback.register("lv_onMacroStopRecording", _onMacroStopRecording);
  Callback.register("lv_onSettingChanged", _onSettingChanged);
  Callback.register("lv_onQuit", _onQuit);
  Callback.register("lv_onUnhandledEscape", _onUnhandledEscape);
  Callback.register("lv_onScroll", _onScroll);
  Callback.register("lv_onStopSearch", _onStopSearch);
  Callback.register("lv_onTerminal", _onTerminal);
  Callback.register("lv_onWindowMovement", _onWindowMovement);
  Callback.register("lv_onWindowSplit", _onWindowSplit);
  Callback.register("lv_onVersion", _onVersion);
  Callback.register("lv_onYank", _onYank);
  Callback.register("lv_onWriteFailure", _onWriteFailure);
  Callback.register("lv_onCursorMoveScreenLine", _onCursorMoveScreenLine);
  Callback.register(
    "lv_onCursorMoveScreenPosition",
    _onCursorMoveScreenPosition,
  );
  Callback.register("lv_onInputMap", _onInputMap);
  Callback.register("lv_onInputUnmap", _onInputUnmap);
  Callback.register("lv_onToggleComments", _onToggleComments);
  Callback.register("lv_onGetChar", _onGetChar);
  Callback.register("lv_onOutput", _onOutput);
  Callback.register("lv_onCursorAdd", _onCursorAdd);

  Native.vimInit();

  BufferInternal.checkCurrentBufferForUpdate();
};

let inputCommon = (~inputFn, ~context=Context.current(), v: string) => {
  let {autoClosingPairs, mode, _}: Context.t = context;
  let cursors = Mode.cursors(mode);
  let ranges = Mode.ranges(mode);
  runWith(
    ~context,
    () => {
      // Special auto-closing pairs handling...

      let runInsertCursor = (cursor: BytePosition.t) => {
        let lineNumber = EditorCoreTypes.LineNumber.toOneBased(cursor.line);
        Undo.saveRegion(lineNumber - 1, lineNumber + 1);
        let originalCursor = cursor;
        Cursor.set(cursor);
        let originalLine = Buffer.getLine(Buffer.getCurrent(), cursor.line);
        if (Mode.current() |> Mode.isInsert) {
          let position: BytePosition.t = Cursor.get();
          let line = Buffer.getLine(Buffer.getCurrent(), position.line);

          let isBetweenClosingPairs = () => {
            AutoClosingPairs.isBetweenClosingPairs(
              line,
              position.byte,
              autoClosingPairs,
            );
          };

          let canCloseBefore = () =>
            AutoClosingPairs.canCloseBefore(
              line,
              position.byte,
              autoClosingPairs,
            );

          if (v == "<BS>"
              && AutoClosingPairs.isBetweenDeletionPairs(
                   line,
                   position.byte,
                   autoClosingPairs,
                 )) {
            Native.vimKey("<DEL>");
            Native.vimKey("<BS>");
          } else if (v == "<CR>" && isBetweenClosingPairs()) {
            let precedingWhitespace =
              Internal.getPrecedingWhitespace(
                ~max=position.byte |> ByteIndex.toInt,
                line,
              );
            Native.vimKey("<CR>");
            Native.vimKey("<CR>");
            Native.vimKey("<UP>");
            if (String.length(precedingWhitespace) > 0) {
              Native.vimInput(precedingWhitespace);
            };
            Native.vimKey("<TAB>");
          } else if (AutoClosingPairs.isPassThrough(
                       v,
                       line,
                       position.byte,
                       autoClosingPairs,
                     )) {
            // Join undo
            Native.vimKey("<C-g>");
            Native.vimKey("U");
            Native.vimKey("<RIGHT>");
          } else if (AutoClosingPairs.isOpeningPair(v, autoClosingPairs)
                     && canCloseBefore()) {
            let pair = AutoClosingPairs.getByOpeningPair(v, autoClosingPairs);
            Native.vimInput(v);
            Native.vimInput(pair.closing);
            // Join undo
            Native.vimKey("<C-g>");
            Native.vimKey("U");
            Native.vimKey("<LEFT>");
          } else {
            inputFn(v);
          };
        } else {
          inputFn(v);
        };
        let newCursor = Cursor.get();
        let delta =
          if (LineNumber.equals(newCursor.line, originalCursor.line)) {
            let newLine = Buffer.getLine(Buffer.getCurrent(), newCursor.line);
            // We need to take the byte-position-delta here,
            // because the cursor may have moved less characters than
            // were actually changed (ie, auto-closing pairs: {|})
            String.length(newLine) - String.length(originalLine);
          } else {
            0;
          };
        (newCursor, delta);
      };

      let mode = Mode.current();
      let cursors = Internal.getDefaultCursors(cursors);
      if (Mode.isInsert(mode)) {
        // Run first command, verify we don't go back to normal mode
        switch (cursors) {
        | [hd, ...tail] =>
          let (newHead, primaryCursorDelta) = runInsertCursor(hd);

          let adjustCursors =
              (
                allCursorDeltas: list((BytePosition.t, int)),
                position: BytePosition.t,
              ) => {
            allCursorDeltas
            |> List.fold_left(
                 (cursor: BytePosition.t, candidateOffset) => {
                   let (positionToConsider: BytePosition.t, deltaBytes) = candidateOffset;
                   if (LineNumber.equals(cursor.line, positionToConsider.line)
                       && ByteIndex.(positionToConsider.byte < cursor.byte)) {
                     {
                       line: cursor.line,
                       byte: ByteIndex.(cursor.byte + deltaBytes),
                     };
                   } else {
                     position;
                   };
                 },
                 position,
               );
          };

          let newMode = Mode.current();
          // If we're still in insert mode, run the command for all the rest of the characters too
          if (Mode.isInsert(newMode)) {
            let revCompare = (a, b) => BytePosition.compare(a, b) * (-1);
            let secondaryCursorAndDeltas =
              tail
              |> List.sort(revCompare)
              |> List.map(adjustCursors([(hd, primaryCursorDelta)]))
              |> List.map(runInsertCursor);

            let secondaryCursors = secondaryCursorAndDeltas |> List.map(fst);

            let allCursors =
              [newHead, ...secondaryCursors]
              |> List.map(adjustCursors(secondaryCursorAndDeltas));

            Insert({cursors: allCursors});
          } else {
            newMode;
          };

        // This should never happen...
        | [] => Insert({cursors: cursors})
        };
      } else if (Mode.isSelect(mode)) {
        // Handle multiple selector cursors
        let maybeEditRange =
          (
            switch (mode) {
            | Select({ranges}) => List.nth_opt(ranges, 0)
            | _ => None
            }
          )
          |> Option.map(VisualRange.range)
          |> Option.map(ByteRange.normalize);

        // Check if we can run multi-cursor insert - preconditions are:
        // 1) All selections are character-wise
        let allSelectionsCharacterWise =
          ranges
          |> List.for_all((range: VisualRange.t) =>
               range.visualType == Types.Character
             );

        // 2) All selections individually span a single line
        let allSelectionsAreSingleLine =
          ranges
          |> List.for_all((range: VisualRange.t) =>
               range.cursor.line == range.anchor.line
             );

        let originalCursor = Cursor.get();
        let originalLine =
          Buffer.getLine(Buffer.getCurrent(), originalCursor.line);

        // Run the cursor as normal...
        switch (cursors) {
        | [hd, ..._] => Cursor.set(hd)
        | _ => ()
        };
        inputFn(v);

        let newCursor = Cursor.get();
        let newLine = Buffer.getLine(Buffer.getCurrent(), newCursor.line);

        let cursorStayedOnSameLine = originalCursor.line == newCursor.line;

        let hasEditRange = maybeEditRange != None;

        // In subsequent iterations, would be nice to remove this limitation..
        let canDoMultiCursorSelect =
          allSelectionsAreSingleLine
          && allSelectionsCharacterWise
          && cursorStayedOnSameLine
          && hasEditRange;

        if (!canDoMultiCursorSelect) {
          Mode.current();
        } else {
          // Apply multi-selection behavior:
          // 1) Shift the secondary ranges by the edit range (the delta between the cursor-selection and inserted text)
          // 2) Replace all the secondary ranges with the inserted text
          // 3) Set up insert mode cursor positions

          let editRange = Option.get(maybeEditRange);

          let byteDelta =
            String.length(newLine) - String.length(originalLine);

          let shiftIfImpactedByPrimaryCursorEdit = (range: ByteRange.t) =>
            // If the cursor shifted, adjust ranges on the same lines
            if (range.start.line == newCursor.line
                && range.start.byte > newCursor.byte) {
              ByteRange.{
                start: {
                  line: range.start.line,
                  byte: ByteIndex.(range.start.byte + byteDelta),
                },
                stop: {
                  line: range.stop.line,
                  byte: ByteIndex.(range.stop.byte + byteDelta),
                },
              };
            } else {
              range;
            };

          let insertedText =
            String.sub(
              newLine,
              ByteIndex.toInt(editRange.start.byte),
              ByteIndex.toInt(editRange.stop.byte)
              + byteDelta
              - ByteIndex.toInt(editRange.start.byte)
              + 1,
            );

          // Filter out all ranges that intersected with the cursor
          let secondaryRanges =
            ranges
            |> List.map(range => VisualRange.range(range))
            |> List.map(ByteRange.normalize)
            |> List.filter(range =>
                 !ByteRange.contains(originalCursor, range)
               )
            |> List.map(shiftIfImpactedByPrimaryCursorEdit);

          let buffer = Buffer.getCurrent();

          // We sort edits in descending order, so we can apply them
          // without adjusting positions. However, we'll need to account
          // for updated positions when mapping to cursors.
          let revCompare = (a, b) => ByteRange.compare(a, b) * (-1);

          // Update subsequent selections - replace current text with insert text
          secondaryRanges
          |> Base.List.sort(~compare=revCompare)
          |> List.iter(range => {
               open EditorCoreTypes.ByteRange;

               let currentLine =
                 Buffer.getLine(Buffer.getCurrent(), range.start.line);

               let updatedLine =
                 replace(
                   ~insert=insertedText,
                   ~start=ByteIndex.toInt(range.start.byte),
                   ~len=
                     ByteIndex.toInt(range.stop.byte)
                     - ByteIndex.toInt(range.start.byte),
                   currentLine,
                 );

               let lineNumber =
                 EditorCoreTypes.LineNumber.toOneBased(range.start.line);
               Undo.saveRegion(lineNumber - 1, lineNumber + 1);
               // Clear out range, and replace with current line
               Buffer.setLines(
                 ~start=range.start.line,
                 ~stop=EditorCoreTypes.LineNumber.(range.start.line + 1),
                 ~lines=[|updatedLine|],
                 buffer,
               );
             });

          // Remap the ranges from the 'source' text position to the range
          // after we've applied all edits.
          let adjustPositionBasedOnPreviousRanges = (allRanges, position) => {
            allRanges
            |> List.fold_left(
                 (curr: BytePosition.t, rangeToConsider: ByteRange.t) =>
                   // If the line is equal, and the range to consider is _before_
                   // our current range, shift the current range by the delta bytes.
                   if (LineNumber.equals(
                         curr.line,
                         rangeToConsider.start.line,
                       )
                       && ByteIndex.(rangeToConsider.start.byte < curr.byte)) {
                     let originalByteLength =
                       ByteIndex.toInt(rangeToConsider.stop.byte)
                       - ByteIndex.toInt(rangeToConsider.start.byte)
                       + 1;
                     let delta =
                       String.length(insertedText) - originalByteLength;
                     {line: curr.line, byte: ByteIndex.(curr.byte + delta)};
                   } else {
                     curr;
                   },
                 position,
               );
          };

          let cursors =
            secondaryRanges
            |> List.map((range: ByteRange.t) => range.start)
            |> List.map(
                 adjustPositionBasedOnPreviousRanges(secondaryRanges),
               )
            |> List.map((pos: BytePosition.t) => {
                 BytePosition.{
                   line: pos.line,
                   byte: ByteIndex.(pos.byte + String.length(insertedText)),
                 }
               });

          // Primary cursor may also need to be updated,
          // if there were select ranges before it on the same line
          let primaryCursor =
            newCursor |> adjustPositionBasedOnPreviousRanges(secondaryRanges);

          // Transition to insert mode, with multiple cursors for each range
          Mode.Insert({cursors: [primaryCursor, ...cursors]});
        };
      } else {
        switch (cursors) {
        | [hd, ..._] => Cursor.set(hd)
        | _ => ()
        };
        inputFn(v);
        Mode.current();
      };
    },
  );
};

let input = inputCommon(~inputFn=Native.vimInput);
let key = inputCommon(~inputFn=Native.vimKey);

module Registers = {
  let get = (~register) => Native.vimRegisterGet(int_of_char(register));
};

let command = (~context=Context.current(), v) => {
  runWith(
    ~context,
    () => {
      Native.vimCommand(v);
      Mode.current();
    },
  );
};

let eval = (~context=Context.current(), v) =>
  // Error messages come through the message handler,
  // so we'll temporarily override it during the course of the eval
  if (v == "") {
    Ok("");
  } else {
    let lastMessage = ref(None);

    GlobalState.overriddenMessageHandler :=
      Some((_priority, _title, msg) => {lastMessage := Some(msg)});
    GlobalState.context := Some(context);
    let maybeEval = Native.vimEval(v);
    GlobalState.context := None;
    GlobalState.overriddenMessageHandler := None;

    switch (maybeEval, lastMessage^) {
    | (Some(eval), _) => Ok(eval)
    | (None, Some(msg)) => Error(msg)
    | (None, None) => Error("Unknown error evaluating " ++ v)
    };
  };

let onDirectoryChanged = f => {
  Event.add(f, Listeners.directoryChanged);
};

let onEffect = f => {
  Event.add(f, Listeners.effect);
};

let onIntro = f => {
  Event.add(f, Listeners.intro);
};

let onMessage = f => {
  Event.add3(f, Listeners.message);
};

let onTerminal = f => {
  Event.add(f, Listeners.terminalRequested);
};

let onQuit = f => {
  Event.add2(f, Listeners.quit);
};

let onUnhandledEscape = f => {
  Event.add(f, Listeners.unhandledEscape);
};

let onVersion = f => {
  Event.add(f, Listeners.version);
};

let onYank = f => {
  Event.add(f, Listeners.yank);
};

let onWriteFailure = f => {
  Event.add2(f, Listeners.writeFailure);
};
