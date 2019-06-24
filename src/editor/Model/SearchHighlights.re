/*
 * SearchHighlights.re
 *
 * Per-buffer highlight info
 */

open Oni_Core;
open Oni_Core.Types;

type matchingPair = {
  startPos: Position.t,
  endPos: Position.t,
};

type highlights = {matchingPair: option(matchingPair)};

type t = IntMap.t(highlights);

let create: unit => t = () => IntMap.empty;

let reduce = (action: Actions.t, state: t) => {
  switch (action) {
  | SearchSetMatchingPair(bid, startPos, endPos) =>
    IntMap.update(
      bid,
      oldHighlights =>
        switch (oldHighlights) {
        | None
        | Some(_) => Some({matchingPair: Some({startPos, endPos})})
        },
      state,
    )
  | SearchClearMatchingPair(bid) =>
    IntMap.update(
      bid,
      oldHighlights =>
        switch (oldHighlights) {
        | None => None
        | Some(_) => Some({matchingPair: None})
        },
      state,
    )
  | _ => state
  };
};
