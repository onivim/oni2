open EditorCoreTypes;

type lineEnding = Types.lineEnding;

module AutoClosingPairs = AutoClosingPairs;
module AutoCommands = AutoCommands;
module AutoIndent = AutoIndent;
module Buffer = Buffer;
module BufferMetadata = BufferMetadata;
module BufferUpdate = BufferUpdate;
module Clipboard = Clipboard;
module CommandLine = CommandLine;
module Context = Context;
module Cursor = Cursor;
module Edit = Edit;
module Effect = Effect;
module Event = Event;
module Format = Format;
module Goto = Goto;
module Mode = Mode;
module Options = Options;
module Search = Search;
module Types = Types;
module Undo = Undo;
module Visual = Visual;
module VisualRange = VisualRange;
module Window = Window;
module Yank = Yank;

module GlobalState = {
  let autoIndent:
    ref(
      option(
        (~previousLine: string, ~beforePreviousLine: option(string)) =>
        AutoIndent.action,
      ),
    ) =
    ref(None);
  let queuedFunctions: ref(list(unit => unit)) = ref([]);
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
          startLine: Index.fromOneBased(startLine),
          endLine: Index.fromOneBased(endLine),
        });
      };
    };

  let getDefaultCursors = (cursors: list(Cursor.t)) =>
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

  context.lineComment |> Option.iter(Options.setLineComment);

  let oldBuf = Buffer.getCurrent();
  let prevMode = Mode.getCurrent();
  let prevLocation = Cursor.getLocation();
  let prevRange = Visual.getRange();
  let prevTopLine = Window.getTopLine();
  let prevLeftColumn = Window.getLeftColumn();
  let prevVisualMode = Visual.getType();
  let prevModified = Buffer.isModified(oldBuf);
  let prevLineEndings = Buffer.getLineEndings(oldBuf);

  GlobalState.autoIndent := Some(context.autoIndent);

  let cursors = f();

  GlobalState.autoIndent := None;

  let newBuf = Buffer.getCurrent();
  let newLocation = Cursor.getLocation();
  let newMode = Mode.getCurrent();
  let newRange = Visual.getRange();
  let newLeftColumn = Window.getLeftColumn();
  let newTopLine = Window.getTopLine();
  let newVisualMode = Visual.getType();
  let newModified = Buffer.isModified(newBuf);
  let newLineEndings = Buffer.getLineEndings(newBuf);

  BufferInternal.checkCurrentBufferForUpdate();

  if (newMode != prevMode) {
    Event.dispatch(newMode, Listeners.modeChanged);

    if (newMode == CommandLine) {
      Event.dispatch(
        CommandLineInternal.getState(),
        Listeners.commandLineEnter,
      );
    } else if (prevMode == CommandLine) {
      Event.dispatch((), Listeners.commandLineLeave);
    };
  } else if (newMode == CommandLine) {
    Event.dispatch(
      CommandLineInternal.getState(),
      Listeners.commandLineUpdate,
    );
  };

  if (!Location.(prevLocation == newLocation)) {
    Event.dispatch(newLocation, Listeners.cursorMoved);
  };

  if (prevTopLine != newTopLine) {
    Event.dispatch(newTopLine, Listeners.topLineChanged);
  };

  if (prevLeftColumn != newLeftColumn) {
    Event.dispatch(newLeftColumn, Listeners.leftColumnChanged);
  };

  if (!Range.equals(prevRange, newRange)
      || newMode == Visual
      && prevMode != Visual
      || prevVisualMode != newVisualMode) {
    let vr =
      VisualRange.create(~range=newRange, ~visualType=newVisualMode, ());
    Event.dispatch(vr, Listeners.visualRangeChanged);
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
  let outContext = {
    ...Context.current(),
    cursors,
    autoClosingPairs: context.autoClosingPairs,
  };
  outContext;
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
  queue(() => Event.dispatch3(priority, title, message, Listeners.message));
};

let _onQuit = (q, f) => {
  queue(() => Event.dispatch2(q, f, Listeners.quit));
};

let _onWindowMovement = (mt, c) => {
  queue(() => Event.dispatch2(mt, c, Listeners.windowMovement));
};

let _onWindowSplit = (st, p) => {
  queue(() => Event.dispatch2(st, p, Listeners.windowSplit));
};

let _onWindowSplit = (st, p) => {
  queue(() => Event.dispatch2(st, p, Listeners.windowSplit));
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
  queue(() => Event.dispatch((), Listeners.stopSearchHighlight));
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
      lnum - 2 |> Index.fromOneBased |> Buffer.getLine(buf) |> Option.some;
    } else {
      None;
    };

  let beforeLine =
    if (lnum >= 2 && lnum <= lineCount) {
      lnum - 1 |> Index.fromOneBased |> Buffer.getLine(buf);
    } else {
      ""; // This should never happen... but follow the Vim convention for empty lines.
    };

  let indentAction =
    GlobalState.autoIndent^
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

  // The [indentOffset] is computed to offset the difference between the previous line and source line,
  // to normalize the indentation provided by the callback function.
  let indentOffset =
    if (aboveWhitespace > afterWhitespace) {
      1;
    } else if (aboveWhitespace < afterWhitespace) {
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

let _onGoto = (_line: int, _column: int, gotoType: Goto.effect) => {
  queue(() => Event.dispatch(Effect.Goto(gotoType), Listeners.effect));
};

let _onTerminal = terminalRequest => {
  queue(() => Event.dispatch(terminalRequest, Listeners.terminalRequested));
};

let _onVersion = () => {
  queue(() => Event.dispatch((), Listeners.version));
};

let init = () => {
  Callback.register("lv_clipboardGet", _clipboardGet);
  Callback.register("lv_onBufferChanged", _onBufferChanged);
  Callback.register("lv_onAutocommand", _onAutocommand);
  Callback.register("lv_onAutoIndent", _onAutoIndent);
  Callback.register("lv_onDirectoryChanged", _onDirectoryChanged);
  Callback.register("lv_onFormat", _onFormat);
  Callback.register("lv_onGoto", _onGoto);
  Callback.register("lv_onIntro", _onIntro);
  Callback.register("lv_onMessage", _onMessage);
  Callback.register("lv_onQuit", _onQuit);
  Callback.register("lv_onUnhandledEscape", _onUnhandledEscape);
  Callback.register("lv_onStopSearch", _onStopSearch);
  Callback.register("lv_onTerminal", _onTerminal);
  Callback.register("lv_onWindowMovement", _onWindowMovement);
  Callback.register("lv_onWindowSplit", _onWindowSplit);
  Callback.register("lv_onVersion", _onVersion);
  Callback.register("lv_onYank", _onYank);
  Callback.register("lv_onWriteFailure", _onWriteFailure);

  Native.vimInit();

  Event.dispatch(Mode.getCurrent(), Listeners.modeChanged);
  BufferInternal.checkCurrentBufferForUpdate();
};

let input = (~context=Context.current(), v: string) => {
  let {autoClosingPairs, cursors, _}: Context.t = context;
  runWith(
    ~context,
    () => {
      // Special auto-closing pairs handling...

      let runCursor = cursor => {
        Cursor.set(cursor);
        if (Mode.getCurrent() == Types.Insert) {
          let location = Cursor.getLocation();
          let line = Buffer.getLine(Buffer.getCurrent(), location.line);

          let isBetweenClosingPairs = () => {
            AutoClosingPairs.isBetweenClosingPairs(
              line,
              location.column,
              autoClosingPairs,
            );
          };

          let canCloseBefore = () =>
            AutoClosingPairs.canCloseBefore(
              line,
              location.column,
              autoClosingPairs,
            );

          if (v == "<BS>"
              && AutoClosingPairs.isBetweenDeletionPairs(
                   line,
                   location.column,
                   autoClosingPairs,
                 )) {
            Native.vimInput("<DEL>");
            Native.vimInput("<BS>");
          } else if (v == "<CR>" && isBetweenClosingPairs()) {
            let precedingWhitespace =
              Internal.getPrecedingWhitespace(
                ~max=location.column |> Index.toOneBased,
                line,
              );
            Native.vimInput("<CR>");
            Native.vimInput("<CR>");
            Native.vimInput("<UP>");
            if (String.length(precedingWhitespace) > 0) {
              Native.vimInput(precedingWhitespace);
            };
            Native.vimInput("<TAB>");
          } else if (AutoClosingPairs.isPassThrough(
                       v,
                       line,
                       location.column,
                       autoClosingPairs,
                     )) {
            Native.vimInput("<RIGHT>");
          } else if (AutoClosingPairs.isOpeningPair(v, autoClosingPairs)
                     && canCloseBefore()) {
            let pair = AutoClosingPairs.getByOpeningPair(v, autoClosingPairs);
            Native.vimInput(v);
            Native.vimInput(pair.closing);
            Native.vimInput("<LEFT>");
          } else {
            Native.vimInput(v);
          };
        } else {
          Native.vimInput(v);
        };
        Cursor.get();
      };

      let mode = Mode.getCurrent();
      let cursors = Internal.getDefaultCursors(cursors);
      if (mode == Types.Insert) {
        // Run first command, verify we don't go back to normal mode
        switch (cursors) {
        | [hd, ...tail] =>
          let newHead = runCursor(hd);

          let newMode = Mode.getCurrent();
          // If we're still in insert mode, run the command for all the rest of the characters too
          let remainingCursors =
            switch (newMode) {
            | Types.Insert => List.map(runCursor, tail)
            | _ => tail
            };

          [newHead, ...remainingCursors];
        // This should never happen...
        | [] => cursors
        };
      } else {
        switch (cursors) {
        | [hd, ..._] => Cursor.set(hd)
        | _ => ()
        };
        Native.vimInput(v);
        Internal.getDefaultCursors([]);
      };
    },
  );
};

let command = v => {
  runWith(
    ~context=Context.current(),
    () => {
      Native.vimCommand(v);
      [];
    },
  );
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
