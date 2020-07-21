/*
 * BufferHighlights.re
 *
 * This module is responsible for managing 'buffer highlights' -
 * document highlights from the language service
 */

open Oni_Model;
open Oni_Syntax;

let reduce = (state: BufferHighlights.t, action: Actions.t) => {
  switch (action) {
  //| EditorCursorMove(_) => BufferHighlights.clearDocumentHighlights(bid, state);
  | BufferHighlights(bufferHighlightAction) =>
    switch (bufferHighlightAction) {
    | DocumentHighlightsAvailable(bid, ranges) =>
      BufferHighlights.setDocumentHighlights(bid, ranges, state)
    | DocumentHighlightsCleared(bid) =>
      BufferHighlights.clearDocumentHighlights(bid, state)
    }
  | SearchSetHighlights(bid, ranges) =>
    BufferHighlights.setSearchHighlights(bid, ranges, state)
  | SearchClearHighlights(bid) =>
    BufferHighlights.clearSearchHighlights(bid, state)
  | _ => state
  };
};
