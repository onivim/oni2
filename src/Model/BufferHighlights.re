/*
 * BufferHighlights.re
 *
 * This module is responsible for managing 'buffer highlights' -
 * document highlights from the language service
 */

open EditorCoreTypes;
open Oni_Core;
open Oni_Core.Utility;

type matchingPair = (Location.t, Location.t);

[@deriving show({with_path: false})]
type action =
  | DocumentHighlightsAvailable(int, [@opaque] list(Range.t))
  | DocumentHighlightsCleared(int);

type highlights = {
  matchingPair: option(matchingPair),
  searchHighlightsByLine: IntMap.t(list(Range.t)),
  documentHighlightsByLine: IntMap.t(list(Range.t)),
};

let default: highlights = {
  matchingPair: None,
  searchHighlightsByLine: IntMap.empty,
  documentHighlightsByLine: IntMap.empty,
};

type t = IntMap.t(highlights);

let initial = IntMap.empty;

let setMatchingPair = (bufferId, loc0, loc1, state) => {
  IntMap.update(
    bufferId,
    oldHighlights =>
      switch (oldHighlights) {
      | None => Some({...default, matchingPair: Some((loc0, loc1))})
      | Some(v) => Some({...v, matchingPair: Some((loc0, loc1))})
      },
    state,
  );
};

let getMatchingPair = (bufferId, state: t) => {
  IntMap.find_opt(bufferId, state)
  |> Option.bind(highlights => highlights.matchingPair);
};

let clearMatchingPair = (bufferId, state: t) => {
  IntMap.update(
    bufferId,
    oldHighlights =>
      switch (oldHighlights) {
      | None => Some(default)
      | Some(v) => Some({...v, matchingPair: None})
      },
    state,
  );
};

let setSearchHighlights = (bufferId, ranges, state) => {
  let searchHighlightsByLine = Utility.RangeUtil.toLineMap(ranges);

  IntMap.update(
    bufferId,
    oldHighlights =>
      switch (oldHighlights) {
      | None => Some({...default, searchHighlightsByLine})
      | Some(v) => Some({...v, searchHighlightsByLine})
      },
    state,
  );
};

let getSearchHighlights = (~bufferId, ~line, state) => {
  IntMap.find_opt(bufferId, state)
  |> Option.map(highlights => highlights.searchHighlightsByLine)
  |> Option.bind(IntMap.find_opt(Index.toZeroBased(line)))
  |> Option.value(~default=[]);
};

let clearSearchHighlights = (bufferId, state) => {
  IntMap.update(
    bufferId,
    fun
    | None => Some(default)
    | Some(v) => Some({...v, searchHighlightsByLine: IntMap.empty}),
    state,
  );
};

let setDocumentHighlights = (bufferId, ranges, state) => {
  let documentHighlightsByLine = Utility.RangeUtil.toLineMap(ranges);

  IntMap.update(
    bufferId,
    oldHighlights =>
      switch (oldHighlights) {
      | None => Some({...default, documentHighlightsByLine})
      | Some(v) => Some({...v, documentHighlightsByLine})
      },
    state,
  );
};

let getDocumentHighlights = (~bufferId, ~line, state) => {
  IntMap.find_opt(bufferId, state)
  |> Option.map(highlights => highlights.documentHighlightsByLine)
  |> Option.bind(IntMap.find_opt(Index.toZeroBased(line)))
  |> Option.value(~default=[]);
};

let clearDocumentHighlights = (bufferId, state) => {
  IntMap.update(
    bufferId,
    fun
    | None => Some(default)
    | Some(v) => Some({...v, documentHighlightsByLine: IntMap.empty}),
    state,
  );
};

let getHighlightsByLine = (~bufferId, ~line, state) => {
  let searchHighlights = getSearchHighlights(~bufferId, ~line, state);
  let documentHighlights = getDocumentHighlights(~bufferId, ~line, state);
  searchHighlights @ documentHighlights;
};

let getHighlights = (~bufferId, state) => {
  let bindingToIndex = binding => {
    let (line, _) = binding;
    Index.fromZeroBased(line);
  };

  let intMapToIndices = intMap =>
    List.map(bindingToIndex, IntMap.bindings(intMap));

  IntMap.find_opt(bufferId, state)
  |> Option.map(highlights => {
       let mergedMap =
         IntMap.union(
           (_key, a, _b) => Some(a),
           highlights.documentHighlightsByLine,
           highlights.searchHighlightsByLine,
         );
       intMapToIndices(mergedMap);
     })
  |> Option.value(~default=[]);
};
