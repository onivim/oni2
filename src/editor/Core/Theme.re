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
    minimapHighlightBackground: Color.t,
  };

  let default: t = {
    background: Color.hex("#282C35"),
    foreground: Color.hex("#ECEFF4"),
    editorBackground: Color.hex("#2F3440"),
    editorForeground: Color.hex("#DCDCDC"),
    editorLineHighlightBackground: Color.hex("#495162"),
    editorLineNumberBackground: Color.hex("#2F3440"),
    editorLineNumberForeground: Color.hex("#495162"),
    editorActiveLineNumberForeground: Color.hex("#737984"),
    scrollbarSliderBackground: Color.rgba(0., 0., 0., 0.2),
    scrollbarSliderActiveBackground: Color.hex("#2F3440"),
    editorMenuBackground: Color.hex("#2F3440"),
    editorMenuForeground: Color.hex("#FFFFFF"),
    editorMenuItemSelected: Color.hex("#495162"),
    tabActiveForeground: Color.hex("#DCDCDC"),
    minimapHighlightBackground: Color.hex("#2C313C"),
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

let getTokenColor = (_theme: t, _scopes: list(string)) => {
  Colors.white;
};

let create: unit => t = () => {colors: EditorColors.default, tokenColors: []};
