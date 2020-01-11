open EditorCoreTypes;
open Oni_Core;
open CamomileBundled.Camomile;

// A [meet] is information about where a completion should be requested
// (where the candidate completions _meet_ the text). It includes
// the position where completions should be requested from, as well
// as the 'base' - candidate text that should apply to the filtering of completion items.
type t = {
  bufferId: int,
  // Base is the prefix string
  base: string,
  // Meet is the location where we request completions
  location: Location.t,
};

let toString: t => string;

let fromLine:
  (
    ~triggerCharacters: list(UChar.t)=?,
    ~lineNumber: int=?,
    ~bufferId: int,
    ~index: Index.t,
    string
  ) =>
  option(t);

let fromBufferLocation:
  (~triggerCharacters: list(UChar.t)=?, ~location: Location.t, Buffer.t) =>
  option(t);
