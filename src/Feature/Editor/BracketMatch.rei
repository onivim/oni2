open EditorCoreTypes;
open Oni_Core;

type pair = {
  start: CharacterPosition.t,
  stop: CharacterPosition.t,
};

let find:
  (
    ~buffer: EditorBuffer.t,
    ~characterPosition: CharacterPosition.t,
    ~start: Uchar.t,
    ~stop: Uchar.t
  ) =>
  option(pair);

let findFirst:
  (
    ~buffer: EditorBuffer.t,
    ~characterPosition: CharacterPosition.t,
    ~pairs: list(LanguageConfiguration.BracketPair.t)
  ) =>
  option(pair);
