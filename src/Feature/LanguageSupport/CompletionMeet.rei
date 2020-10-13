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
  // Meet is the location where we request completions
  location: CharacterPosition.t,
};

let matches: (t, t) => bool;

let toString: t => string;

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
