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
      Configuration.getValue(
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
    maybeLanguageConfig
    |> OptionEx.flatMap((config: LanguageConfiguration.t) =>
         config.lineComment
       );
  };

  let indentation = (~buffer) =>
    buffer
    |> OptionEx.flatMap(Buffer.getIndentation)
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

  Vim.Context.{
    autoIndent,
    bufferId,
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
