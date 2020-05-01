open Oni_Core;
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

  let decode = obj(({field, _}) => {
        let startLineNumber = field.required( "startLineNumber", int);
        let endLineNumber = field.required( "endLineNumber", int);
        let startColumn = field.required( "startColumn", int);
        let endColumn = field.required( "endColumn", int);
        let severity = field.required( "severity", int);
        let message = field.required( "message", string);

        let range = OneBasedRange.create(
          ~startLineNumber,
          ~endLineNumber,
          ~startColumn,
          ~endColumn,
          ()
        );

       { range, severity, message };
  });
};

let decode = Decode.decode;
