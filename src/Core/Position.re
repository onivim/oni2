[@deriving show({with_path: false})]
type t = {
  line: Index.t,
  character: Index.t,
};

let create = (line, character) => {line, character};

let createFromZeroBasedIndices = (line: int, character: int) => {
  line: ZeroBasedIndex(line),
  character: ZeroBasedIndex(character),
};

let ofInt0 = createFromZeroBasedIndices;

let createFromOneBasedIndices = (line: int, character: int) => {
  line: OneBasedIndex(line),
  character: OneBasedIndex(character),
};

let ofInt1 = createFromOneBasedIndices;

let equals = (a: t, b: t) => {
  Index.equals(a.line, b.line) && Index.equals(a.character, b.character);
};