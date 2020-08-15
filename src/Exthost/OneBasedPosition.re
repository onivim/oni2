open EditorCoreTypes;

[@deriving (show({with_path: false}), yojson({strict: false}))]
type t = {
  lineNumber: int,
  column: int,
};

let ofPosition = (p: Location.t) => {
  lineNumber: p.line |> Index.toOneBased,
  column: p.column |> Index.toOneBased,
};
