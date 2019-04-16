/*
 * WhitespaceTokenFilter.re
 *
 * Implements logic for filtering out whitespace tokens
 * based on the user's configuration settings
 */

open Oni_Core;
open BufferViewTokenizer;
open Configuration;

let filterAllWhitespace = token =>
  switch (token.tokenType) {
  | Text => true
  | _ => false
  };

let filterBoundaryWhitespace = token =>
  switch (token.tokenType) {
  | Text => true
  | Tab => true
  | Whitespace => String.length(token.text) > 1
  };

let filter =
    (
      whitespaceSetting: Configuration.editorRenderWhitespace,
      tokens: list(BufferViewTokenizer.t),
    ) => {
  switch (whitespaceSetting) {
  | All => tokens
  | Boundary => List.filter(filterBoundaryWhitespace, tokens)
  | None => List.filter(filterAllWhitespace, tokens)
  };
};
