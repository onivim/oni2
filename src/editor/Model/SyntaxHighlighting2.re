/*
 * SyntaxHighlighting2.re
 *
 * State kept for syntax highlighting (via TextMate today)
 */
open Oni_Core;
open Oni_Core.Types;
open Oni_Syntax.SyntaxHighlights;

type t = IntMap.t(SyntaxHighlights.t);

let empty = IntMap.empty;
