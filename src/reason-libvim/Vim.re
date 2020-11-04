open EditorCoreTypes;

type lineEnding = Types.lineEnding;

module AutoClosingPairs = AutoClosingPairs;
module AutoCommands = AutoCommands;
module AutoIndent = AutoIndent;
module Buffer = Buffer;
module BufferMetadata = BufferMetadata;
module BufferUpdate = BufferUpdate;
module Clipboard = Clipboard;
module ColorScheme = ColorScheme;
module CommandLine = CommandLine;
module Context = Context;
module Cursor = Cursor;
module Edit = Edit;
module Effect = Effect;
module Event = Event;
module Format = Format;
module Goto = Goto;
module Mapping = Mapping;
module Operator = Operator;
module Scroll = Scroll;
module TabPage = TabPage;
module Mode = Mode;
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

  let oldBuf = Buffer.getCurrent();
  let prevMode = Mode.trySet(context.mode);
  let prevModified = Buffer.isModified(oldBuf);
  let prevLineEndings = Buffer.getLineEndings(oldBuf);

  GlobalState.autoIndent := Some(context.autoIndent);
  GlobalState.colorSchemeProvider := context.colorSchemeProvider;
  GlobalState.viewLineMotion := Some(context.viewLineMotion);
  GlobalState.screenPositionMotion := Some(context.screenCursorMotion);
  GlobalState.effects := [];
  GlobalState.toggleComments := Some(context.toggleComments);

  let mode = f();

  GlobalState.autoIndent := None;
  GlobalState.colorSchemeProvider := ColorScheme.Provider.default;
  GlobalState.viewLineMotion := None;
  GlobalState.screenPositionMotion := None;
  GlobalState.toggleComments := None;

  let newBuf = Buffer.getCurrent();
  let newMode = Mode.current();
  let newModified = Buffer.isModified(newBuf);
  let newLineEndings = Buffer.getLineEndings(newBuf);

  BufferInternal.checkCurrentBufferForUpdate();

  if (newMode != prevMode) {
    Event.dispatch(Effect.ModeChanged(newMode), Listeners.effect);

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
  queue(() => Event.dispatch(Effect.Goto(gotoType), Listeners.effect));
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
  GlobalState.colorSchemeProvider^(pattern);
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

let init = () => {
  Callback.register("lv_clipboardGet", _clipboardGet);
  Callback.register("lv_onBufferChanged", _onBufferChanged);
  Callback.register("lv_onAutocommand", _onAutocommand);
  Callback.register("lv_onAutoIndent", _onAutoIndent);
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

  Native.vimInit();

  Event.dispatch(Effect.ModeChanged(Mode.current()), Listeners.effect);
  BufferInternal.checkCurrentBufferForUpdate();
};

let inputCommon = (~inputFn, ~context=Context.current(), v: string) => {
  let {autoClosingPairs, mode, _}: Context.t = context;
  let cursors = Mode.cursors(mode);
  runWith(
    ~context,
    () => {
      // Special auto-closing pairs handling...

      let runCursor = cursor => {
        Cursor.set(cursor);
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
        Cursor.get();
      };

      let mode = Mode.current();
      let cursors = Internal.getDefaultCursors(cursors);
      if (Mode.isInsert(mode)) {
        // Run first command, verify we don't go back to normal mode
        switch (cursors) {
        | [hd, ...tail] =>
          let newHead = runCursor(hd);

          let newMode = Mode.current();
          // If we're still in insert mode, run the command for all the rest of the characters too
          if (Mode.isInsert(newMode)) {
            let remainingCursors = List.map(runCursor, tail);
            Insert({cursors: [newHead, ...remainingCursors]});
          } else {
            newMode;
          };

        // This should never happen...
        | [] => Insert({cursors: cursors})
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

let eval = v =>
  // Error messages come through the message handler,
  // so we'll temporarily override it during the course of the eval
  if (v == "") {
    Ok("");
  } else {
    let lastMessage = ref(None);

    GlobalState.overriddenMessageHandler :=
      Some((_priority, _title, msg) => {lastMessage := Some(msg)});

    let maybeEval = Native.vimEval(v);

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
