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
  Json.Decode.
    // The default `int` decoder, or `Number` json type,
    // doesn't handle exponents. So we need to use the float decoder / type,
    // and coerce it to an int.
    (
      {
        let intWithExponent =
          float
          |> map(f
               // If the value is outside of `Int.max_int` or `Int.min_int`,
               // `float_of_int` returns 0:
               // https://stackoverflow.com/questions/48871692/how-to-handle-integer-overflow-in-ocaml-when-converting-from-float-using-int-of
               =>
                 if (f >= float_of_int(Int.max_int)) {
                   Int.max_int;
                 } else if (f <= float_of_int(Int.min_int)) {
                   Int.min_int;
                 } else {
                   int_of_float(f);
                 }
               );
        obj(({field, _}) =>
          {
            startLineNumber:
              field.required("startLineNumber", intWithExponent),
            endLineNumber: field.required("endLineNumber", intWithExponent),
            startColumn: field.required("startColumn", intWithExponent),
            endColumn: field.required("endColumn", intWithExponent),
          }
        );
      }
    );

let%test_module "decode" =
  (module
   {
     let%test "#2820 - Number.MAX_VALUE handling" = {
       let json = {|
   {
    "startLineNumber": 0,
    "startColumn": 0,
    "endLineNumber": 1.7976931348623157e+308,
    "endColumn": 1.7976931348623157e+308
   }
  |};

       let range =
         json
         |> Yojson.Safe.from_string
         |> Json.Decode.decode_value(decode)
         |> Result.get_ok;

       range
       == {
            startLineNumber: 0,
            startColumn: 0,
            endLineNumber: Int.max_int,
            endColumn: Int.max_int,
          };
     };
   });

let encode = range =>
  Json.Encode.(
    obj([
      ("startLineNumber", range.startLineNumber |> int),
      ("endLineNumber", range.endLineNumber |> int),
      ("startColumn", range.startColumn |> int),
      ("endColumn", range.endColumn |> int),
    ])
  );
