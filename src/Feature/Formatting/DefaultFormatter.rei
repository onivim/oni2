open EditorCoreTypes;
open Oni_Core;

// DefaultFormatter is used as a fall-back formatter when
// a formatter is not provided by a language extension.

// [format(~indentation, ~languageConfiguration, lines)] returns
// a set of edits to correctly indent the provided [lines], based on
// [indentation] and [languageConfiguration] settings.
let format:
  (
    ~indentation: IndentationSettings.t,
    ~languageConfiguration: LanguageConfiguration.t,
    ~startLineNumber: Index.t,
    list(string)
  ) =>
  list(Vim.Edit.t);
