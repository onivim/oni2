module Colors = Feature_Theme.Colors.Terminal;
let theme = theme =>
  fun
  | 0 => Colors.ansiBlack.from(theme)
  | 1 => Colors.ansiRed.from(theme)
  | 2 => Colors.ansiGreen.from(theme)
  | 3 => Colors.ansiYellow.from(theme)
  | 4 => Colors.ansiBlue.from(theme)
  | 5 => Colors.ansiMagenta.from(theme)
  | 6 => Colors.ansiCyan.from(theme)
  | 7 => Colors.ansiWhite.from(theme)
  | 8 => Colors.ansiBrightBlack.from(theme)
  | 9 => Colors.ansiBrightRed.from(theme)
  | 10 => Colors.ansiBrightGreen.from(theme)
  | 11 => Colors.ansiBrightYellow.from(theme)
  | 12 => Colors.ansiBrightBlue.from(theme)
  | 13 => Colors.ansiBrightMagenta.from(theme)
  | 14 => Colors.ansiBrightCyan.from(theme)
  | 15 => Colors.ansiBrightWhite.from(theme)
  // For 256 colors, fall back to defaults
  | idx => EditorTerminal.Theme.default(idx);

let defaultBackground = theme => Colors.background.from(theme);
let defaultForeground = theme => Colors.foreground.from(theme);
