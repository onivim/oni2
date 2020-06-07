open EditorCoreTypes;

type lineEnding = Types.lineEnding;

module AutoClosingPairs = AutoClosingPairs;
module AutoCommands = AutoCommands;
module Buffer = Buffer;
module BufferMetadata = BufferMetadata;
module BufferUpdate = BufferUpdate;
module Clipboard = Clipboard;
module CommandLine = CommandLine;
module Context = Context;
module Cursor = Cursor;
module Effect = Effect;
module Event = Event;
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

type fn = unit => unit;

let queuedFunctions: ref(list(fn)) = ref([]);

let queue = f => queuedFunctions := [f, ...queuedFunctions^];

let flushQueue = () => {
  queuedFunctions^ |> List.rev |> List.iter(f => f());

  queuedFunctions := [];
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

  let cursors = f();

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

let _onGoto = (_line: int, _column: int, gotoType: Goto.t) => {
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
  Callback.register("lv_onDirectoryChanged", _onDirectoryChanged);
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

let _getDefaultCursors = (cursors: list(Cursor.t)) =>
  if (cursors == []) {
    [Cursor.get()];
  } else {
    cursors;
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
            Native.vimInput("<CR>");
            Native.vimInput("<CR>");
            Native.vimInput("<UP>");
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
      let cursors = _getDefaultCursors(cursors);
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
        _getDefaultCursors([]);
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
