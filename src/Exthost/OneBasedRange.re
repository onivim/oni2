open EditorCoreTypes;
open Oni_Core;

[@deriving (show({with_path: false}), yojson({strict: false, exn: true}))]
type t = {
  startLineNumber: int,
  endLineNumber: int,
  startColumn: int,
  endColumn: int,
};

let create = (~startLineNumber, ~endLineNumber, ~startColumn, ~endColumn, ()) => {
  startLineNumber,
  endLineNumber,
  startColumn,
  endColumn,
};

let one = {
  startLineNumber: 1,
  endLineNumber: 1,
  startColumn: 1,
  endColumn: 1,
};

let ofRange = (r: CharacterRange.t) => {
  startLineNumber: r.start.line |> EditorCoreTypes.LineNumber.toOneBased,
  endLineNumber: r.stop.line |> EditorCoreTypes.LineNumber.toOneBased,
  startColumn: (r.start.character |> CharacterIndex.toInt) + 1,
  endColumn: (r.stop.character |> CharacterIndex.toInt) + 1,
};

let toRange = ({startLineNumber, endLineNumber, startColumn, endColumn}) => {
  EditorCoreTypes.(
    CharacterRange.{
      start:
        EditorCoreTypes.CharacterPosition.{
          line: LineNumber.ofOneBased(startLineNumber),
          character: CharacterIndex.ofInt(startColumn - 1),
        },
      stop:
        EditorCoreTypes.CharacterPosition.{
          line: LineNumber.ofOneBased(endLineNumber),
          character: CharacterIndex.ofInt(endColumn - 1),
        },
    }
  );
};

let decode =
  Json.Decode.(
    {
      obj(({field, _}) =>
        {
          startLineNumber: field.required("startLineNumber", int),
          endLineNumber: field.required("endLineNumber", int),
          startColumn: field.required("startColumn", int),
          endColumn: field.required("endColumn", int),
        }
      );
    }
  );

let encode = range =>
  Json.Encode.(
    obj([
      ("startLineNumber", range.startLineNumber |> int),
      ("endLineNumber", range.endLineNumber |> int),
      ("startColumn", range.startColumn |> int),
      ("endColumn", range.endColumn |> int),
    ])
  );
