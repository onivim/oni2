[@deriving show({with_path: false})]
type t = {
  line: Index.t,
  column: Index.t,
};

let create = (~line, ~column) => {line, column};

let equals = (a, b) => Index.(a.line == b.line && a.column == b.column);

let (==) = equals;

let toString = ({line, column}) =>
  Printf.sprintf("%n,%n", (line :> int), (column :> int));
