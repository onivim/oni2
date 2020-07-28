/*
 * BufferHighlights.re
 *
 * This module is responsible for managing 'buffer highlights' -
 * document highlights from the language service
 */

open EditorCoreTypes;
open Oni_Core;
open Oni_Core.Utility;

type highlights = {searchHighlightsByLine: IntMap.t(list(Range.t))};

let default: highlights = {searchHighlightsByLine: IntMap.empty};

type t = IntMap.t(highlights);

let initial = IntMap.empty;

let setSearchHighlights = (bufferId, ranges, state) => {
  let searchHighlightsByLine = RangeEx.toLineMap(ranges);

  IntMap.add(
    bufferId,
    {searchHighlightsByLine: searchHighlightsByLine},
    state,
  );
};

let getSearchHighlights = (~bufferId, ~line, state) => {
  IntMap.find_opt(bufferId, state)
  |> Option.map(highlights => highlights.searchHighlightsByLine)
  |> OptionEx.flatMap(IntMap.find_opt(Index.toZeroBased(line)))
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

//let setDocumentHighlights = (bufferId, ranges, state) => {
//  let documentHighlightsByLine = RangeEx.toLineMap(ranges);
//
//  IntMap.update(
//    bufferId,
//    oldHighlights =>
//      switch (oldHighlights) {
//      | None => Some({...default, documentHighlightsByLine})
//      | Some(v) => Some({...v, documentHighlightsByLine})
//      },
//    state,
//  );
//};

//let getDocumentHighlights = (~bufferId, ~line, state) => {
//  IntMap.find_opt(bufferId, state)
//  |> Option.map(highlights => highlights.documentHighlightsByLine)
//  |> OptionEx.flatMap(IntMap.find_opt(Index.toZeroBased(line)))
//  |> Option.value(~default=[]);
//};

//let clearDocumentHighlights = (bufferId, state) => {
//  IntMap.update(
//    bufferId,
//    fun
//    | None => Some(default)
//    | Some(v) => Some({ documentHighlightsByLine: IntMap.empty}),
//    state,
//  );
//};

let getHighlightsByLine = (~bufferId, ~line, state) => {
  let searchHighlights = getSearchHighlights(~bufferId, ~line, state);
  //  let documentHighlights = getDocumentHighlights(~bufferId, ~line, state);
  //  searchHighlights @ documentHighlights;
  searchHighlights;
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
       //       let mergedMap =
       //         IntMap.union(
       //           (_key, a, _b) => Some(a),
       //           //highlights.documentHighlightsByLine,
       //           highlights.searchHighlightsByLine,
       //         );
       highlights.searchHighlightsByLine
       |> intMapToIndices
     })
  |> Option.value(~default=[]);
};
