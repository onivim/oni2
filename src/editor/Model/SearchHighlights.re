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

type highlights = {
	matchingPair: option(matchingPair),
	highlightRanges: list(Range.t)
};

let default: highlights = {
	matchingPair: None,
	highlightRanges: [],
};

type t = IntMap.t(highlights);

let create: unit => t = () => IntMap.empty;

let reduce = (action: Actions.t, state: t) => {
  switch (action) {
  | SearchSetMatchingPair(bid, startPos, endPos) =>
    IntMap.update(
      bid,
      oldHighlights =>
        switch (oldHighlights) {
        | None => Some({...default, matchingPair: Some({startPos, endPos})})
        | Some(v) => Some({...v, matchingPair: Some({startPos, endPos})})
        },
      state,
    )
  | SearchSetHighlights(bid, highlightRanges) =>
  	IntMap.update(
		bid,
		oldHighlights =>
		switch(oldHighlights) {
		| None => Some({...default, highlightRanges })
		| Some(v) => Some({...v, highlightRanges })
		}, state);
  | SearchClearMatchingPair(bid) =>
    IntMap.update(
      bid,
      oldHighlights =>
        switch (oldHighlights) {
        | None => None
        | Some(v) => Some({...v, matchingPair: None})
        },
      state,
    )
  | _ => state
  };
};
