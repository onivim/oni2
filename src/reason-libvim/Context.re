open EditorCoreTypes;

type t = {
  autoClosingPairs: AutoClosingPairs.t,
  autoIndent:
    (~previousLine: string, ~beforePreviousLine: option(string)) =>
    AutoIndent.action,
  bufferId: int,
  colorSchemeProvider: ColorScheme.Provider.t,
  width: int,
  height: int,
  leftColumn: int,
  topLine: int,
  cursors: list(BytePosition.t),
  lineComment: option(string),
  tabSize: int,
  insertSpaces: bool,
};

let current = () => {
  autoClosingPairs: AutoClosingPairs.empty,
  autoIndent: (~previousLine as _, ~beforePreviousLine as _) => AutoIndent.KeepIndent,
  bufferId: Buffer.getCurrent() |> Buffer.getId,
  colorSchemeProvider: ColorScheme.Provider.default,
  width: Window.getWidth(),
  height: Window.getHeight(),
  leftColumn: Window.getLeftColumn(),
  topLine: Window.getTopLine(),
  cursors: [Cursor.get()],
  tabSize: Options.getTabSize(),
  insertSpaces: Options.getInsertSpaces(),
  lineComment: None,
};
