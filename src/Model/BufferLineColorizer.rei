/*
 * BufferViewColorizer.rei
 */

open EditorCoreTypes;
open Revery;
open Oni_Core;

/*
 * Type [tokenColor] is a tuple of [(backgroundColor, foregroundColor)]
 */
type tokenColor = {
  backgroundColor: Color.t,
  foregroundColor: Color.t,
  bold: bool,
  italic: bool,
};

/*
 * Type [t] is a function of [(int) => tokenColor)].
 *
 */
type t = int => tokenColor;

/*
 * [create] takes information about the line, like
 * the syntax highlighting ([tokenColors]), the selection,
 * and others - and consolidates it into a colorizer
 * function [t].
 */
let create:
  (
    ~startIndex: int,
    ~endIndex: int,
    ~defaultBackgroundColor: Color.t,
    ~defaultForegroundColor: Color.t, // theme.editorForeground
    ~selectionHighlights: option(Range.t),
    ~selectionColor: Color.t,
    ~matchingPair: option(int),
    ~searchHighlights: list(Range.t),
    ~searchHighlightColor: Color.t, // theme.editorFindMatchBackground
    list(ColorizedToken.t)
  ) =>
  t;
