[@deriving show({with_path: false})]
type t = {
  line: LineNumber.t,
  character: CharacterIndex.t,
};

let zero: t;

let equals: (t, t) => bool;
let (==): (t, t) => bool;

let isWithinOneCharacter: (t, t) => bool;
