open EditorCoreTypes;
open Oni_Core;
open Oni_Core.Utility;
open Feature_Editor;

module Log = (val Log.withNamespace("Oni2.Model.VimContext"));

module Internal = {
  let syntaxScope = (~maybeCursor: option(BytePosition.t), state: State.t) => {
    state
    |> Selectors.getActiveBuffer
    |> OptionEx.flatMap(buffer => {
         let bufferId = Buffer.getId(buffer);

         maybeCursor
         |> Option.map((cursor: BytePosition.t) => {
              Feature_Syntax.getSyntaxScope(
                ~bufferId,
                ~bytePosition=cursor,
                state.syntaxHighlights,
              )
            });
       })
    |> Option.value(~default=SyntaxScope.none);
  };

  let functionGetChar = (mode: Vim.Functions.GetChar.mode) => {
    Vim.Functions.GetChar.(
      switch (mode) {
      | Immediate =>
        Log.warn("getchar(0) not yet implemented");
        char_of_int(0);
      | Peek =>
        Log.warn("getchar(1) not yet implemented");
        char_of_int(0);
      | Wait =>
        let currentTime = Unix.gettimeofday();
        let char = ref(None);

        // Implement a five-second timeout
        // Perhaps could integrate a 'timeouttlen' configuration setting?
        while (currentTime +. 5. > Unix.gettimeofday() && char^ == None) {
          // Not an ideal implementation of getchar - this busy-waits
          // (and steals SDL events!)
          // Some improvements to be made:
          // - Push this back into the Revery layer, so we can still render in the meantime while busy-waiting (and handle non-keyboard events)
          // Show some
          switch (Sdl2.Event.waitTimeout(100)) {
          | None => ()
          | Some(Sdl2.Event.TextInput({text, _})) =>
            if (String.length(text) == 1) {
              char := Some(text.[0]);
            } else {
              Log.warnf(m =>
                m("getchar - ignoring multi-byte string: %s", text)
              );
            }
          | Some(_) => ()
          };
        };

        char^
        |> OptionEx.tapNone(() => Log.warn("getchar() timed out."))
        |> OptionEx.tap(c => Log.infof(m => m("getchar: Got a key '%c'", c)))
        |> Option.value(~default=char_of_int(0));
      }
    );
  };

  let autoClosingPairs =
      (~fileType, ~syntaxScope, ~maybeLanguageConfig, state: State.t) => {
    let perFileTypeResolver =
      Feature_Configuration.resolver(~fileType, state.config, state.vim);
    let acpEnabled =
      Feature_Editor.Configuration.autoClosingPairs.get(perFileTypeResolver)
      |> (
        fun
        | `LanguageDefined => true
        | `Never => false
      );

    if (acpEnabled) {
      maybeLanguageConfig
      |> Option.map(
           LanguageConfiguration.toVimAutoClosingPairs(syntaxScope),
         )
      |> Option.value(~default=Vim.AutoClosingPairs.empty);
    } else {
      Vim.AutoClosingPairs.empty;
    };
  };

  let lineComment = (~maybeLanguageConfig) => {
    maybeLanguageConfig |> OptionEx.flatMap(LanguageConfiguration.lineComment);
  };

  let indentation = (~buffer) =>
    buffer
    |> Option.map(Buffer.getIndentation)
    |> Option.value(~default=IndentationSettings.default);
};

let current = (state: State.t) => {
  let editor = Feature_Layout.activeEditor(state.layout);
  let bufferId = Editor.getBufferId(editor);
  let mode = Editor.mode(editor);

  let editorBuffer = Selectors.getActiveBuffer(state);
  let fileType =
    editorBuffer
    |> Option.map(buf => Buffer.getFileType(buf) |> Buffer.FileType.toString)
    |> Option.value(~default=Buffer.FileType.default);

  let maybeLanguageConfig: option(LanguageConfiguration.t) =
    editorBuffer
    |> OptionEx.flatMap(buf =>
         Buffer.getFileType(buf) |> Buffer.FileType.toOption
       )
    |> OptionEx.flatMap(
         Exthost.LanguageInfo.getLanguageConfiguration(state.languageInfo),
       );

  let maybeCursor =
    switch (Editor.getCursors(editor)) {
    | [hd, ..._] => Some(hd)
    | [] => None
    };

  // TODO: Hook up to Vim context
  let autoIndent =
    maybeLanguageConfig
    |> Option.map(LanguageConfiguration.toAutoIndent)
    |> Option.value(~default=(~previousLine as _, ~beforePreviousLine as _) =>
         Vim.AutoIndent.KeepIndent
       );

  let syntaxScope = Internal.syntaxScope(~maybeCursor, state);
  let autoClosingPairs =
    Internal.autoClosingPairs(
      ~fileType,
      ~syntaxScope,
      ~maybeLanguageConfig,
      state,
    );

  let Feature_Editor.EditorLayout.{bufferWidthInCharacters: width, _} =
    Editor.getLayout(editor);

  let leftColumn = Editor.getLeftVisibleColumn(editor);
  let topLine =
    Editor.getTopVisibleBufferLine(editor)
    |> EditorCoreTypes.LineNumber.toOneBased;
  let bottomLine =
    Editor.getBottomVisibleBufferLine(editor)
    |> EditorCoreTypes.LineNumber.toOneBased;

  // Set configured line comment
  let lineComment = Internal.lineComment(~maybeLanguageConfig);

  let toggleComments =
    switch (lineComment) {
    | None => (lines => lines)
    | Some(lineComment) => Comments.toggle(~lineComment)
    };
  let indentation = Internal.indentation(~buffer=editorBuffer);

  let insertSpaces = indentation.mode == Spaces;

  let colorSchemeProvider = pattern => {
    state.extensions
    |> Feature_Extensions.themesByName(~filter=pattern)
    |> Array.of_list;
  };

  let viewLineMotion = (~motion, ~count, ~startLine as _) => {
    open EditorCoreTypes;
    let topLine =
      editor |> Editor.getTopVisibleBufferLine |> LineNumber.toZeroBased;
    let bottomLine =
      (editor |> Editor.getBottomVisibleBufferLine |> LineNumber.toZeroBased)
      - 1;
    let normalizedCount = max(count - 1, 0);
    switch (motion) {
    | Vim.ViewLineMotion.MotionH =>
      LineNumber.ofZeroBased(min(topLine + normalizedCount, bottomLine))
    | Vim.ViewLineMotion.MotionM =>
      LineNumber.ofZeroBased(topLine + (bottomLine - topLine) / 2)
    | Vim.ViewLineMotion.MotionL =>
      LineNumber.ofZeroBased(max(bottomLine - normalizedCount, topLine))
    };
  };

  let screenCursorMotion =
      (~direction, ~count, ~line as lnum, ~currentByte, ~wantByte as _) => {
    let count =
      switch (direction) {
      | `Up => count * (-1)
      | `Down => count
      };
    Editor.moveScreenLines(
      ~position=EditorCoreTypes.BytePosition.{line: lnum, byte: currentByte},
      ~count,
      editor,
    );
  };

  Vim.Context.{
    autoIndent,
    bufferId,
    colorSchemeProvider,
    viewLineMotion,
    screenCursorMotion,
    leftColumn,
    topLine,
    width,
    height: bottomLine - topLine,
    mode,
    autoClosingPairs,
    toggleComments,
    insertSpaces,
    subMode: Vim.SubMode.None,
    tabSize: indentation.size,
    functionGetChar: Internal.functionGetChar,
  };
};
