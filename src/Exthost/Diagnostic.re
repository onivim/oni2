open Oni_Core;

module Tag = {
  [@deriving show]
  type t =
    | Unused
    | Deprecated;

  let ofInt =
    fun
    | 1 => Some(Unused)
    | 2 => Some(Deprecated)
    | _ => None;

  let decode =
    Json.Decode.(
      {
        int
        |> and_then(idx => {
             switch (ofInt(idx)) {
             | Some(tag) => succeed(tag)
             | None => fail("Invalid tag: " ++ string_of_int(idx))
             }
           });
      }
    );
};

module Severity = {
  // Must be kept in-sync with:
  // https://github.com/onivim/vscode-exthost/blob/ed480e2fbe01ad85b8d85a2ca2dbd1b85c29242b/src/vs/platform/markers/common/markers.ts#L45
  [@deriving show]
  type t =
    | Hint // 1
    | Info // 2
    | Warning // 4
    | Error; // 8

  let toInt =
    fun
    | Hint => 1
    | Info => 2
    | Warning => 4
    | Error => 8;

  let max = (a, b) => {
    switch (a, b) {
    | (Error, _)
    | (_, Error) => Error
    | (Warning, _)
    | (_, Warning) => Warning
    | (Info, _)
    | (_, Info) => Info
    | _ => Hint
    };
  };

  let ofInt =
    fun
    | 1 => Some(Hint)
    | 2 => Some(Info)
    | 4 => Some(Warning)
    | 8 => Some(Error)
    | _ => None;

  let decode =
    Json.Decode.(
      int
      |> and_then(v => {
           switch (ofInt(v)) {
           | None => fail("Unable to parse severity: " ++ string_of_int(v))
           | Some(sev) => succeed(sev)
           }
         })
    );
};

[@deriving show]
type t = {
  range: OneBasedRange.t,
  message: string,
  severity: Severity.t,
  tags: list(Tag.t),
  // TODO:
  // source: string,
  // code: string,
  // relatedInformation: DiagnosticRelatedInformation.t,
};

module Decode = {
  open Json.Decode;

  let decode =
    obj(({field, _}) => {
      let startLineNumber = field.required("startLineNumber", int);
      let endLineNumber = field.required("endLineNumber", int);
      let startColumn = field.required("startColumn", int);
      let endColumn = field.required("endColumn", int);
      let severity = field.required("severity", Severity.decode);
      let message = field.required("message", string);
      let tags = field.withDefault("tags", [], list(Tag.decode));

      let range =
        OneBasedRange.create(
          ~startLineNumber,
          ~endLineNumber,
          ~startColumn,
          ~endColumn,
          (),
        );

      {range, severity, message, tags};
    });
};

let decode = Decode.decode;
