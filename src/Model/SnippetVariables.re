open EditorCoreTypes;
open Oni_Core;
open Utility;

let current = (state: State.t, variable) => {
  let normalizedVariable = String.uppercase_ascii(variable);

  let editor = Feature_Layout.activeEditor(state.layout);
  let maybeBuffer = Selectors.getActiveBuffer(state);

  let cursorPosition = Feature_Editor.Editor.getPrimaryCursor(editor);

  let lineIdx =
    CharacterPosition.(cursorPosition.line)
    |> EditorCoreTypes.LineNumber.toZeroBased;

  let maybePath = maybeBuffer |> OptionEx.flatMap(Buffer.getFilePath);

  let time = Unix.time();
  let timeInfo = time |> Unix.localtime;

  let maybeLanguage =
    maybeBuffer
    |> Option.map(Buffer.getFileType)
    |> Option.map(Buffer.FileType.toString);

  let maybeLanguageConfiguration =
    maybeLanguage
    |> OptionEx.flatMap(language => {
         Exthost.LanguageInfo.getLanguageConfiguration(
           state.languageSupport |> Feature_LanguageSupport.languageInfo,
           language,
         )
       });

  switch (normalizedVariable) {
  // Textmate Variables

  // TODO
  | "TM_SELECTED_TEXT" =>
    editor
    |> Feature_Editor.Editor.singleLineSelectedText
    |> OptionEx.or_(Some(""))

  | "TM_CURRENT_LINE" =>
    maybeBuffer
    |> OptionEx.flatMap(buffer => {
         let lineCount = Buffer.getNumberOfLines(buffer);

         if (lineIdx >= 0 && lineIdx < lineCount) {
           Some(Buffer.getLine(lineIdx, buffer) |> BufferLine.raw);
         } else {
           None;
         };
       })

  // TODO
  | "TM_CURRENT_WORD" => Some("TM_CURRENT_WORD")

  | "TM_LINE_INDEX" => Some(string_of_int(lineIdx))

  | "TM_LINE_NUMBER" => Some(string_of_int(lineIdx + 1))

  | "TM_FILENAME" => maybePath |> Option.map(Filename.basename)

  | "TM_FILENAME_BASE" =>
    maybePath
    |> Option.map(Filename.basename)
    |> Option.map(Filename.remove_extension)

  | "TM_DIRECTORY" => maybePath |> Option.map(Filename.dirname)

  | "TM_FILEPATH" => maybePath

  // TODO
  | "CLIPBOARD" => Some("CLIPBOARD")

  // DATE / TIME VARIABLES

  | "CURRENT_YEAR" => Some(timeInfo.tm_year + 1900 |> string_of_int)

  // TODO
  | "CURRENT_YEAR_SHORT" => Some("CURRENT_YEAR_SHORT")

  | "CURRENT_MONTH" => Some(timeInfo.tm_mon |> string_of_int)

  // TODO
  | "CURRENT_MONTH_NAME" => Some("CURRENT_MONTH_NAME")

  // TODO
  | "CURRENT_MONTH_NAME_SHORT" => Some("CURRENT_MONTH_NAME_SHORT")

  | "CURRENT_DATE" => Some(timeInfo.tm_mday |> string_of_int)

  // TODO
  | "CURRENT_DAY_NAME" => Some("CURRENT_DAY_NAME")

  // TODO
  | "CURRENT_DAY_NAME_SHORT" => Some("CURRENT_DAY_NAME_SHORT")

  | "CURRENT_HOUR" => Some(timeInfo.tm_hour |> string_of_int)

  | "CURRENT_MINUTE" => Some(timeInfo.tm_min |> string_of_int)

  | "CURRENT_SECOND" => Some(timeInfo.tm_sec |> string_of_int)

  | "CURRENT_SECONDS_UNIX" => Some(time |> int_of_float |> string_of_int)

  | "LINE_COMMENT" =>
    maybeLanguageConfiguration
    |> OptionEx.flatMap(LanguageConfiguration.lineComment)

  | "BLOCK_COMMENT_START" =>
    maybeLanguageConfiguration
    |> OptionEx.flatMap(LanguageConfiguration.blockComment)
    |> Option.map(fst)

  | "BLOCK_COMMENT_END" =>
    maybeLanguageConfiguration
    |> OptionEx.flatMap(LanguageConfiguration.blockComment)
    |> Option.map(snd)

  | _ => None
  };
};
