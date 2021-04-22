open Oni_Core;
type t = {
  selectionStartLineNumber: int,
  selectionStartColumn: int,
  positionLineNumber: int,
  positionColumn: int,
};

let encode = selection => {
  Json.Encode.(
    obj([
      ("selectionStartLineNumber", selection.selectionStartLineNumber |> int),
      ("selectionStartColumn", selection.selectionStartColumn |> int),
      ("positionLineNumber", selection.positionLineNumber |> int),
      ("positionColumn", selection.positionColumn |> int),
    ])
  );
};
