/*
 * BufferSyntaxHighlights.re
 *
 * State kept for per-buffer syntax highlighting
 */
open Oni_Core;
open Oni_Core.Types;
open Oni_Core.Utility;
open Oni_Syntax;

open Revery;

module BufferMap = IntMap;

module LineMap = IntMap;

type t = BufferMap.t(LineMap.t(list(ColorizedToken.t)));

let empty = BufferMap.empty;

let noTokens = [];

let getTokens = (bufferId: int, line: int, highlights: t) => {
 /* highlights
  |> BufferMap.find_opt(bufferId)
  |> Option.bind(LineMap.find_opt(line))
  |> Option.value(~default=noTokens);
*/

  [
    ColorizedToken.create(~index=0, ~backgroundColor=Colors.red, ~foregroundColor=Colors.red, ()),
    ColorizedToken.create(~index=4, ~backgroundColor=Colors.blue, ~foregroundColor=Colors.blue, ()),
    ColorizedToken.create(~index=8, ~backgroundColor=Colors.green, ~foregroundColor=Colors.green, ()),
  ]
};

let setTokens = (bufferId: int, line: int, tokens: list(ColorizedToken.t), highlights: t) => {
  let updateLineMap = (lineMap: LineMap.t(list(ColorizedToken.t))) => {
    LineMap.update(line, (_) => Some(tokens), lineMap);
  };

  BufferMap.update(bufferId, 
    fun
    | None => Some(updateLineMap(LineMap.empty))
    | Some(v) => Some(updateLineMap(v)),
    highlights);
}
