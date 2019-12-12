/*
 * BufferHighlights.re
 *
 * This module is responsible for managing 'buffer highlights' -
 * document highlights from the language service
 */

open Oni_Core;
open Oni_Core.Utility;

type t = IntMap.t(list(Range.t));

let initial: t = IntMap.empty;

let set = (bufferId: int, ranges: list(Range.t), hl: t) => {
  IntMap.update(bufferId, (_) => Some(ranges), hl);
}

let get = (bufferId: int, hl: t) => {
  IntMap.find_opt(bufferId, hl)
  |> Option.value(~default=[]);
}
