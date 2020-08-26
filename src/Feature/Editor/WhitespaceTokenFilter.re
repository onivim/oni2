/*
 * WhitespaceTokenFilter.re
 *
 * Implements logic for filtering out whitespace tokens
 * based on the user's configuration settings
 */

open EditorCoreTypes;
open BufferViewTokenizer;

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

let filterRange = ({start, stop}: ByteRange.t, token) => {
  switch (token.tokenType) {
  | Text => true
  | Tab
  | Whitespace => token.startByte >= start.byte && token.startByte < stop.byte
  };
};

let filter =
    (
      ~selection: option(ByteRange.t),
      whitespaceSetting: [ | `All | `Selection | `Boundary | `None],
      tokens: list(BufferViewTokenizer.t),
    ) => {
  switch (whitespaceSetting) {
  | `All => tokens
  | `Selection =>
    switch (selection) {
    | None => List.filter(filterAllWhitespace, tokens)
    | Some(byteRange) => List.filter(filterRange(byteRange), tokens)
    }
  | `Boundary => List.filter(filterBoundaryWhitespace, tokens)
  | `None => List.filter(filterAllWhitespace, tokens)
  };
};
