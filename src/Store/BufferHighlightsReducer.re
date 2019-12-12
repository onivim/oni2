/*
 * BufferHighlights.re
 *
 * This module is responsible for managing 'buffer highlights' -
 * document highlights from the language service
 */

open Oni_Model;

let reduce = (state: BufferHighlights.t, action: Actions.t) => {
  switch (action) {
  //| EditorCursorMove(_) => BufferHighlights.clearDocumentHighlights(bid, state);
  | SearchSetMatchingPair(bid, startPos, endPos) =>
    BufferHighlights.setMatchingPair(bid, startPos, endPos, state)
  | SearchSetHighlights(bid, ranges) =>
    BufferHighlights.setSearchHighlights(bid, ranges, state)
  | SearchClearHighlights(bid) =>
    BufferHighlights.clearSearchHighlights(bid, state)
  | SearchClearMatchingPair(bid) =>
    BufferHighlights.clearMatchingPair(bid, state)
  | _ => state
  };
};
