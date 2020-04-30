open EditorCoreTypes;
open Oni_Core;
open Oni_Core.Utility;
open Feature_Editor;

module Ext = Oni_Extensions;

module Internal = {
  let syntaxScope = (~cursor: option(Vim.Cursor.t), state: State.t) => {
    state
    |> Selectors.getActiveBuffer
    |> OptionEx.flatMap(buffer => {
         let bufferId = Buffer.getId(buffer);

         cursor
         |> Option.map((cursor: Vim.Cursor.t) => {
              Feature_Syntax.getSyntaxScope(
                ~bufferId,
                ~line=cursor.line,
                ~bytePosition=Index.toZeroBased(cursor.column),
                state.syntaxHighlights,
              )
            });
       })
    |> Option.value(~default=SyntaxScope.none);
  };

  let autoClosingPairs = (~syntaxScope, ~languageConfigLoader, state: State.t) => {
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
      state
      |> Selectors.getActiveBuffer
      |> OptionEx.flatMap(Buffer.getFileType)
      |> OptionEx.flatMap(
           Ext.LanguageConfigurationLoader.get_opt(languageConfigLoader),
         )
      |> Option.map(
           Ext.LanguageConfiguration.toVimAutoClosingPairs(syntaxScope),
         )
      |> Option.value(~default=Vim.AutoClosingPairs.empty);
    } else {
      Vim.AutoClosingPairs.empty;
    };
  };
};

let current:
  (~languageConfigLoader: Ext.LanguageConfigurationLoader.t, State.t) =>
  Vim.Context.t =
  (~languageConfigLoader, state) => {
    state
    |> Selectors.getActiveEditorGroup
    |> Selectors.getActiveEditor
    |> Option.map((editor: Editor.t) => {
         let {bufferId, cursors, _}: Editor.t = editor;

         let maybeCursor =
           switch (Editor.getVimCursors(editor)) {
           | [hd, ..._] => Some(hd)
           | [] => None
           };

         let syntaxScope = Internal.syntaxScope(~cursor=maybeCursor, state);
         let autoClosingPairs =
           Internal.autoClosingPairs(
             ~syntaxScope,
             ~languageConfigLoader,
             state,
           );

         let Feature_Editor.EditorLayout.{
               bufferHeightInCharacters: height,
               bufferWidthInCharacters: width,
               _,
             } =
           Editor.getLayout(editor);

         let leftColumn = Editor.getLeftVisibleColumn(editor);
         let topLine = Editor.getTopVisibleLine(editor);
         let editorBuffer = Selectors.getActiveBuffer(state);

         // Set configured line comment
         let lineComment =
           editorBuffer
           |> OptionEx.flatMap(Buffer.getFileType)
           |> OptionEx.flatMap(
                Ext.LanguageConfigurationLoader.get_opt(languageConfigLoader),
              )
           |> OptionEx.flatMap((config: Ext.LanguageConfiguration.t) =>
                config.lineComment
              );

         let indentation =
           editorBuffer
           |> OptionEx.flatMap(Buffer.getIndentation)
           |> Option.value(~default=IndentationSettings.default);

         let insertSpaces =
           switch (indentation.mode) {
           | Tabs => false
           | Spaces => true
           };

         Vim.Context.{
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
       })
    |> Option.value(~default=Vim.Context.current());
  };
