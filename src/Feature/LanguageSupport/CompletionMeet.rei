open EditorCoreTypes;
open Oni_Core;

// A [meet] is information about where a completion should be requested
// (where the candidate completions _meet_ the text). It includes
// the position where completions should be requested from, as well
// as the 'base' - candidate text that should apply to the filtering of completion items.
[@deriving show]
type t = {
  bufferId: int,
  // Base is the prefix string
  base: string,
  // `location` is the location where we request completions
  location: CharacterPosition.t,
  // `insertLocation` is the location where snippets are keywords should begin insertion
  // Often, this will be overridden by the completion provider
  insertLocation: CharacterPosition.t,
};

let matches: (t, t) => bool;

let toString: t => string;

// For both [fromLine] and [fromBufferPosition], the [index] is the cursor position, which is usually _after_ a character
// that could compose the base. For example, for the line ["console.l|"] where "|" is the cursor position, the expectation
// is that `index` would be called with [9] - the 0-based index _after_ the "l" character.

// This would return a meet at index 7 (the [.]) character, and a base of ["l"].

let shiftMeet: (~edits: list(Exthost.Edit.SingleEditOperation.t), t) => t;

let fromLine:
  (
    ~languageConfiguration: LanguageConfiguration.t,
    ~triggerCharacters: list(Uchar.t)=?,
    ~lineNumber: int=?,
    ~bufferId: int,
    ~index: CharacterIndex.t,
    BufferLine.t
  ) =>
  option(t);

let fromBufferPosition:
  (
    ~languageConfiguration: LanguageConfiguration.t,
    ~triggerCharacters: list(Uchar.t)=?,
    ~position: CharacterPosition.t,
    Buffer.t
  ) =>
  option(t);
