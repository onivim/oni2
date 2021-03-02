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

let moveMarkers = (~newBuffer, ~markerUpdate, model) => {
  let bufferId = Oni_Core.Buffer.getId(newBuffer);
  let shiftLines = (~afterLine, ~delta, searchHighlightsByLine) => {
    let line = EditorCoreTypes.LineNumber.toZeroBased(afterLine);
    searchHighlightsByLine
    |> IntMap.shift(~startPos=line, ~endPos=line, ~delta);
  };

  let shiftCharacters =
      (
        ~line,
        ~afterByte,
        ~deltaBytes,
        ~afterCharacter as _,
        ~deltaCharacters as _,
        searchHighlightsByLine,
      ) => {
    searchHighlightsByLine;
  };

  model
  |> IntMap.update(
       bufferId,
       Option.map(({searchHighlightsByLine}) => {
         let searchHighlightsByLine' =
           MarkerUpdate.apply(
             ~shiftLines,
             ~shiftCharacters,
             markerUpdate,
             searchHighlightsByLine,
           );
         {searchHighlightsByLine: searchHighlightsByLine'};
       }),
     );
};

let getHighlightsByLine = (~bufferId, ~line, state) => {
  let searchHighlights = getSearchHighlights(~bufferId, ~line, state);
  searchHighlights;
};
