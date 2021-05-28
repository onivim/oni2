[@deriving show({with_path: false})]
type t = {
  line: LineNumber.t,
  character: CharacterIndex.t,
};

let isWithinOneCharacter = (a: t, b: t) => {
  a.line == b.line
  && abs(
       CharacterIndex.toInt(a.character) - CharacterIndex.toInt(b.character),
     )
  <= 1;
};

let equals = (a, b) => {
  a.line == b.line && a.character == b.character;
};

let zero = {line: LineNumber.zero, character: CharacterIndex.zero};

let (==) = equals;
