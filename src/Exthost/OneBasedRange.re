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

let ofRange = (r: Range.t) => {
  startLineNumber: r.start.line |> Index.toOneBased,
  endLineNumber: r.stop.line |> Index.toOneBased,
  startColumn: r.start.column |> Index.toOneBased,
  endColumn: r.stop.column |> Index.toOneBased,
};

let toRange = ({startLineNumber, endLineNumber, startColumn, endColumn}) => {
  Range.{
    start:
      Location.{
        line: Index.fromOneBased(startLineNumber),
        column: Index.fromOneBased(startColumn),
      },
    stop:
      Location.{
        line: Index.fromOneBased(endLineNumber),
        column: Index.fromOneBased(endColumn),
      },
  };
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
