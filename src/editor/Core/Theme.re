/*
 * Theme.re
 *
 * Theming color / info
 */

open Revery;

module EditorColors = {
  type t = {
    background: Color.t,
    foreground: Color.t,
    editorBackground: Color.t,
    editorForeground: Color.t,
    editorLineHighlightBackground: Color.t,
    editorLineNumberBackground: Color.t,
    editorLineNumberForeground: Color.t,
    editorActiveLineNumberForeground: Color.t,
    scrollbarSliderBackground: Color.t,
    scrollbarSliderActiveBackground: Color.t,
    editorMenuBackground: Color.t,
    editorMenuForeground: Color.t,
    editorMenuItemSelected: Color.t,
    tabActiveForeground: Color.t,
    statusBarItemVisualModeBackground: Color.t,
    statusBarItemInsertModeBackground: Color.t,
    statusBarItemReplaceModeBackground: Color.t,
    statusBarItemNormalModeBackground: Color.t,
    statusBarItemOperatorModeBackground: Color.t,
    statusBarItemCommandlineModeBackground: Color.t,
    statusBarItemVisualModeForeground: Color.t,
    statusBarItemInsertModeForeground: Color.t,
    statusBarItemReplaceModeForeground: Color.t,
    statusBarItemNormalModeForeground: Color.t,
    statusBarItemOperatorModeForeground: Color.t,
    statusBarItemCommandlineModeForeground: Color.t,
    statusBarItemForeground: Color.t,
    statusBarItemBackground: Color.t,
  };

  let default: t = {
    background: Color.hex("#282C35"),
    foreground: Color.hex("#ECEFF4"),
    editorBackground: Color.hex("#2F3440"),
    editorForeground: Color.hex("#DCDCDC"),
    editorLineHighlightBackground: Color.hex("#2C313C"),
    editorLineNumberBackground: Color.hex("#2F3440"),
    editorLineNumberForeground: Color.hex("#495162"),
    editorActiveLineNumberForeground: Color.hex("#737984"),
    scrollbarSliderBackground: Color.rgba(0., 0., 0., 0.2),
    scrollbarSliderActiveBackground: Color.hex("#2F3440"),
    editorMenuBackground: Color.hex("#2F3440"),
    editorMenuForeground: Color.hex("#FFFFFF"),
    editorMenuItemSelected: Color.hex("#495162"),
    tabActiveForeground: Color.hex("#DCDCDC"),
    statusBarItemVisualModeBackground: Color.hex("#56b6c2"),
    statusBarItemInsertModeBackground: Color.hex("#98c379"),
    statusBarItemReplaceModeBackground: Color.hex("#d19a66"),
    statusBarItemNormalModeBackground: Color.hex("#61afef"),
    statusBarItemOperatorModeBackground: Color.hex("#d19a66"),
    statusBarItemCommandlineModeBackground: Color.hex("#61afef"),
    statusBarItemVisualModeForeground: Color.hex("#282c34"),
    statusBarItemInsertModeForeground: Color.hex("#282c34"),
    statusBarItemReplaceModeForeground: Color.hex("#282c34"),
    statusBarItemNormalModeForeground: Color.hex("#282c34"),
    statusBarItemOperatorModeForeground: Color.hex("#282c34"),
    statusBarItemCommandlineModeForeground: Color.hex("#282c34"),
    statusBarItemBackground: Color.hex("#495162"),
    statusBarItemForeground: Color.hex("#fff"),
  };
};

module TokenColor = {
  type fontStyle =
    | Normal
    | Bold
    | Italic;

  type settings = {
    foreground: option(Color.t),
    fontStyle: option(fontStyle),
  };

  type t = {
    name: string,
    scope: list(string),
    settings,
  };
};

type t = {
  colors: EditorColors.t,
  tokenColors: list(TokenColor.t),
};

let getTokenColor = (_theme: t, _scopes: list(string)) => Colors.white;

let create: unit => t = () => {colors: EditorColors.default, tokenColors: []};
