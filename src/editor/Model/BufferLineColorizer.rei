/*
 * BufferViewColorizer.rei
 */

open Revery;
open Oni_Core;
open Oni_Core.Types;
open Oni_Extensions;

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
    int,
    Theme.t,
    list(ColorizedToken2.t),
    option(Range.t),
    Color.t,
    Color.t,
    option(int),
    list(Range.t)
  ) =>
  t;
