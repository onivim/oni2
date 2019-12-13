/*
 * BufferViewColorizer.rei
 */

open EditorCoreTypes;
open Revery;
open Oni_Core;

/*
 * Type [tokenColor] is a tuple of [(backgroundColor, foregroundColor)]
 */
type tokenColor = (Color.t, Color.t);

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
    Theme.t,
    list(ColorizedToken.t),
    option(Range.t),
    Color.t,
    Color.t,
    option(int),
    list(Range.t)
  ) =>
  t;
