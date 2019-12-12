open EditorCoreTypes;
open Oni_Core;
open CamomileBundled.Camomile;

// A [meet] is information about where a completion should be requested
// (where the candidate completions _meet_ the text). It includes
// the position where completions should be requested from, as well
// as the 'base' - candidate text that should apply to the filtering of completion items.
type t;

let toString: t => string;

// [getLocation(meet)] returns a [Position.t] where completions should be
// requested from.
let getLocation: t => Location.t;

// [getBase(meet)] returns a string of the 'base' - the characters
// prefixing the completion. These can be used for filtering completion
// items.
let getBase: t => string;

let create: (~location: Location.t, ~base: string) => t;

let createFromLine:
  (
    ~triggerCharacters: list(UChar.t)=?,
    ~lineNumber: int=?,
    ~index: Index.t,
    string
  ) =>
  option(t);

let createFromBufferLocation:
  (~triggerCharacters: list(UChar.t)=?, ~location: Location.t, Buffer.t) =>
  option(t);
