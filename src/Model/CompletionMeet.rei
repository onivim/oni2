open Oni_Core;
open CamomileBundled.Camomile;

// A [meet] is information about where a completion should be requested
// (where the candidate completions _meet_ the text). It includes
// the position where completions should be requested from, as well
// as the 'base' - candidate text that should apply to the filtering of completion items.
type t;

let toString: t => string;

// [getPosition(meet)] returns a [Position.t] where completions should be
// requested from.
let getPosition: t => Position.t;

// [getBase(meet)] returns a string of the 'base' - the characters
// prefixing the completion. These can be used for filtering completion
// items.
let getBase: t => string;

let create: (~position: Position.t, ~base: string) => t;

let createFromLine:
  (
    ~triggerCharacters: list(UChar.t)=?,
    ~lineNumber: int=?,
    ~index: Index.t,
    string
  ) =>
  option(t);

let createFromBufferPosition:
  (~triggerCharacters: list(UChar.t)=?, ~position: Position.t, Buffer.t) =>
  option(t);
