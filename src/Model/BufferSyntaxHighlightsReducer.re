/*
 * BufferSyntaxHighlightsReducer.re
 *
 * State kept for per-buffer syntax highlighting
 */
open Oni_Core;
open Oni_Core.Types;
open Oni_Core.Utility;
open Oni_Syntax;

let reduce = (state: BufferSyntaxHighlights.t, action: Actions.t) => {
  switch (action) {
  | BufferSyntaxHighlights(tokens) =>
    BufferSyntaxHighlights.setTokens(tokens, state)
  | _ => state
  };
};
