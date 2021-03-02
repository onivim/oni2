/*
 * BufferHighlights.re
 *
 * This module is responsible for managing 'buffer highlights' -
 * document highlights from the language service
 */

open EditorCoreTypes;
open Oni_Core;
open Oni_Core.Utility;

type highlights = {searchHighlightsByLine: IntMap.t(list(ByteRange.t))};

let default: highlights = {searchHighlightsByLine: IntMap.empty};

type t = IntMap.t(highlights);

let initial = IntMap.empty;

let setSearchHighlights = (bufferId, ranges, state) => {
  let searchHighlightsByLine = RangeEx.toByteLineMap(ranges);

  IntMap.add(
    bufferId,
    {searchHighlightsByLine: searchHighlightsByLine},
    state,
  );
};

let getSearchHighlights = (~bufferId, ~line, state) => {
  IntMap.find_opt(bufferId, state)
  |> Option.map(highlights => highlights.searchHighlightsByLine)
  |> OptionEx.flatMap(
       IntMap.find_opt(EditorCoreTypes.LineNumber.toZeroBased(line)),
     )
  |> Option.value(~default=[]);
};

let clearSearchHighlights = (bufferId, state) => {
  IntMap.update(
    bufferId,
    fun
    | None => Some(default)
    | Some(_) => Some({searchHighlightsByLine: IntMap.empty}),
    state,
  );
};

let getHighlightsByLine = (~bufferId, ~line, state) => {
  let searchHighlights = getSearchHighlights(~bufferId, ~line, state);
  searchHighlights;
};
