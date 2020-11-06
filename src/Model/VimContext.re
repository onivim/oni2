open EditorCoreTypes;
open Oni_Core;
open Oni_Core.Utility;
open Feature_Editor;

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

  let autoClosingPairs = (~syntaxScope, ~maybeLanguageConfig, state: State.t) => {
    let acpEnabled =
      Oni_Core.Configuration.getValue(
        c => c.editorAutoClosingBrackets,
        state.configuration,
      )
      |> (
        fun
        | LanguageDefined => true
        | Never => false
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
    Internal.autoClosingPairs(~syntaxScope, ~maybeLanguageConfig, state);

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

  let viewLineMotion = (~motion, ~count as _, ~startLine as _) => {
    switch (motion) {
    | Vim.ViewLineMotion.MotionH => Editor.getTopVisibleBufferLine(editor)
    | Vim.ViewLineMotion.MotionM =>
      EditorCoreTypes.(
        {
          let topLine =
            editor |> Editor.getTopVisibleBufferLine |> LineNumber.toZeroBased;
          let bottomLine =
            editor
            |> Editor.getBottomVisibleBufferLine
            |> LineNumber.toZeroBased;
          LineNumber.ofZeroBased(topLine + (bottomLine - topLine) / 2);
        }
      )
    | Vim.ViewLineMotion.MotionL =>
      EditorCoreTypes.LineNumber.(
        Editor.getBottomVisibleBufferLine(editor) - 1
      )
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
    tabSize: indentation.size,
  };
};
