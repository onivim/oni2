open Oni_Core;
type position = {
  line: int,
  index: int,
};

type pair = {
  start: position,
  stop: position,
};

let find:
  (
    ~buffer: EditorBuffer.t,
    ~line: int,
    ~index: int,
    ~start: Uchar.t,
    ~stop: Uchar.t
  ) =>
  option(pair);

let findFirst:
  (
    ~buffer: EditorBuffer.t,
    ~line: int,
    ~index: int,
    ~pairs: list(LanguageConfiguration.BracketPair.t)
  ) =>
  option(pair);
