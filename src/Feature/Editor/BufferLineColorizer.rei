/*
 * BufferViewColorizer.rei
 */

open EditorCoreTypes;
open Revery;
open Oni_Core;

/*
 * Type [tokenTheme] is a record containing theme information
 */
type tokenTheme = {
  color: Color.t,
  backgroundColor: Color.t,
  italic: bool,
  bold: bool,
};

/*
 * Type [t] is a function of [(int) => tokenTheme)].
 *
 */
type t = int => tokenTheme;

/*
 * [create] takes information about the line, like
 * the syntax highlighting ([tokenThemes]), the selection,
 * and others - and consolidates it into a colorizer
 * function [t].
 */
let create:
  (
    ~startByte: int,
    ~endByte: int,
    ~defaultBackgroundColor: Color.t,
    ~defaultForegroundColor: Color.t, // theme.editorForeground
    ~selectionHighlights: option(Range.t),
    ~selectionColor: Color.t,
    ~matchingPair: option(int),
    ~searchHighlights: list(Range.t),
    ~searchHighlightColor: Color.t, // theme.editorFindMatchBackground
    list(ThemeToken.t)
  ) =>
  t;
