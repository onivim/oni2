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
  let cursors = Editor.getCursors(editor);

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

  let Feature_Editor.EditorLayout.{
        bufferHeightInCharacters: height,
        bufferWidthInCharacters: width,
        _,
      } =
    // TODO: Fix this
    Editor.getLayout(~showLineNumbers=true, ~maxMinimapCharacters=0, editor);

  let leftColumn = Editor.getLeftVisibleColumn(editor);
  let topLine = Editor.getTopVisibleLine(editor);

  // Set configured line comment
  let lineComment = Internal.lineComment(~maybeLanguageConfig);

  let indentation = Internal.indentation(~buffer=editorBuffer);

  let insertSpaces = indentation.mode == Spaces;

  let colorSchemeProvider = pattern => {
    state.extensions
    |> Feature_Extensions.themesByName(~filter=pattern)
    |> Array.of_list;
  };

  let viewLineMotion = (~motion, ~count as _, ~startLine as _) => {
    switch (motion) {
    | Vim.ViewLineMotion.MotionH =>
      Editor.getTopVisibleLine(editor)
      |> EditorCoreTypes.LineNumber.ofZeroBased
    | Vim.ViewLineMotion.MotionM =>
      Editor.getTopVisibleLine(editor)
      + (
        Editor.getBottomVisibleLine(editor)
        - Editor.getTopVisibleLine(editor)
      )
      / 2
      |> EditorCoreTypes.LineNumber.ofZeroBased
    | Vim.ViewLineMotion.MotionL =>
      Editor.getBottomVisibleLine(editor)
      |> EditorCoreTypes.LineNumber.ofZeroBased
    };
  };

  let screenCursorMotion =
      (~direction, ~count, ~line as lnum, ~currentByte as _, ~wantByte) => {
    let maxLines = Editor.getBufferLineCount(editor);
    let lnum = EditorCoreTypes.LineNumber.toZeroBased(lnum);
    let destLineIdx =
      switch (direction) {
      | `Up => lnum - count
      | `Down => lnum + count
      };

    let destLine =
      IntEx.clamp(~hi=maxLines - 1, ~lo=0, destLineIdx)
      |> EditorCoreTypes.LineNumber.ofZeroBased;
    BytePosition.{line: destLine, byte: wantByte};
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
    height,
    cursors,
    autoClosingPairs,
    lineComment,
    insertSpaces,
    tabSize: indentation.size,
  };
};
