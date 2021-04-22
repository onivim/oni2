[@deriving show]
type t = {
  start: CharacterIndex.t,
  stop: CharacterIndex.t,
};

let zero = {start: CharacterIndex.zero, stop: CharacterIndex.zero};

let toRange = (~line, span: t) => {
  CharacterRange.{
    start: CharacterPosition.{line, character: span.start},
    stop: CharacterPosition.{line, character: span.stop},
  };
};
