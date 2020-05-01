[@deriving yojson({strict: false})]
type json = {
  startLineNumber: int,
  endLineNumber: int,
  startColumn: int,
  endColumn: int,
  message: string,
  severity: int,
  // TODO:
  // source: string,
  //code: string,
  // relatedInformation: DiagnosticRelatedInformation.t,
};

type t = {
  range: OneBasedRange.t,
  message: string,
  severity: int,
  // TODO:
  // source: string,
  // code: string,
  // relatedInformation: DiagnosticRelatedInformation.t,
};
module Decode = {
  open Json.Decode;

  let decode = string |> map(StringEx.explode);

  let configuration =
    obj(({field, at, _}) =>
      {
        startLineNumber: field.required( "startLineNumber", int),
        endLineNumber: field.required( "startLineNumber", int),
        startColumn: field.required( "startColumn", int),
        autoClosingPairs:
          field.withDefault(
            "autoClosingPairs",
            [],
            list(AutoClosingPair.decode),
          ),
        lineComment: at.optional(["comments", "lineComment"], string),
        blockComment:
          at.optional(
            ["comments", "blockComment"],
            list(string)
            |> and_then(
                 fun
                 | [start, stop] => succeed((start, stop))
                 | _ => fail("Expected pair"),
               ),
          ),
      }
    );
};

let of_yojson = json => {
  switch (json_of_yojson(json)) {
  | Ok(ret) =>
    let range =
      OneBasedRange.create(
        ~startLineNumber=ret.startLineNumber,
        ~startColumn=ret.startColumn,
        ~endLineNumber=ret.endLineNumber,
        ~endColumn=ret.endColumn,
        (),
      );

    Ok({
      range,
      message: ret.message,
      severity: ret.severity,
      // TODO:
      //source: ret.source,
      //code: ret.code,
    });
  | Error(msg) => Error(msg)
  };
};

let to_yojson = _ => {
  // TODO:
  failwith("Not used");
};
