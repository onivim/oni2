[@deriving show({with_path: false})]
type t = {
  line: LineNumber.t,
  character: CharacterIndex.t,
};

let equals = (a, b) => {
  a.line == b.line && a.character == b.character;
};

let zero = {line: LineNumber.zero, character: CharacterIndex.zero};

let (==) = equals;
