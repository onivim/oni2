open EditorCoreTypes;

[@deriving (show({with_path: false}), yojson({strict: false}))]
type t = {
  lineNumber: int,
  column: int,
};

let ofPosition = (p: CharacterPosition.t) => {
  lineNumber: p.line |> LineNumber.toOneBased,
  column: (p.character |> CharacterIndex.toInt) + 1,
};
