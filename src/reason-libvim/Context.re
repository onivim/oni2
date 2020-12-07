open EditorCoreTypes;

type t = {
  autoClosingPairs: AutoClosingPairs.t,
  autoIndent:
    (~previousLine: string, ~beforePreviousLine: option(string)) =>
    AutoIndent.action,
  viewLineMotion:
    (~motion: ViewLineMotion.t, ~count: int, ~startLine: LineNumber.t) =>
    LineNumber.t,
  screenCursorMotion:
    (
      ~direction: [ | `Up | `Down],
      ~count: int,
      ~line: LineNumber.t,
      ~currentByte: ByteIndex.t,
      ~wantByte: ByteIndex.t
    ) =>
    BytePosition.t,
  toggleComments: array(string) => array(string),
  bufferId: int,
  colorSchemeProvider: ColorScheme.Provider.t,
  width: int,
  height: int,
  leftColumn: int,
  topLine: int,
  mode: Mode.t,
  subMode: SubMode.t,
  tabSize: int,
  insertSpaces: bool,
  functionGetChar: Functions.GetChar.t,
};

let current = () => {
  autoClosingPairs: AutoClosingPairs.empty,
  autoIndent: (~previousLine as _, ~beforePreviousLine as _) => AutoIndent.KeepIndent,
  viewLineMotion: (~motion as _, ~count as _, ~startLine) => startLine,
  screenCursorMotion:
    (~direction as _, ~count as _, ~line, ~currentByte as _, ~wantByte) => {
    line,
    byte: wantByte,
  },
  bufferId: Buffer.getCurrent() |> Buffer.getId,
  colorSchemeProvider: ColorScheme.Provider.default,
  width: Window.getWidth(),
  height: Window.getHeight(),
  leftColumn: Window.getLeftColumn(),
  topLine: Window.getTopLine(),
  mode: Mode.current(),
  subMode: Native.vimGetSubMode(),
  tabSize: Options.getTabSize(),
  insertSpaces: Options.getInsertSpaces(),
  toggleComments: lines => lines,
  functionGetChar: _ => char_of_int(0),
};
