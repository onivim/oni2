open EditorCoreTypes;

type t = {
  name: string,
  detail: string,
  kind: SymbolKind.t,
  //TODO: containerName?
  range: Range.t,
  //TODO: selectionRange?
  children: list(t),
};

let create = (~children=[], ~name, ~detail, ~kind, ~range) => {
  name,
  detail,
  kind,
  range,
  children,
};
