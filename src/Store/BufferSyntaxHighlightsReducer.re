/*
 * BufferSyntaxHighlightsReducer.re
 *
 * State kept for per-buffer syntax highlighting
 */

open Oni_Model;

module BufferSyntaxHighlights = Feature_Editor.BufferSyntaxHighlights;

let reduce = (state: BufferSyntaxHighlights.t, action: Actions.t) => {
  switch (action) {
  | BufferSyntaxHighlights(tokens) =>
    BufferSyntaxHighlights.setTokens(tokens, state)
  | BufferUpdate(bu) when !bu.update.isFull =>
    BufferSyntaxHighlights.handleUpdate(bu.update, state)
  | _ => state
  };
};
