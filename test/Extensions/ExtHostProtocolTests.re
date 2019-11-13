open Oni_Core;
open Oni_Core_Test;
open Oni_Extensions;

open ExtHostProtocol;

open TestFramework;

let diagnosticCollectionJSON = {|
["test",[[{"$mid":1,"fsPath":"/Users/bryphe/oni2/test.oni-dev","external":"file:/Users/bryphe/oni2/test.oni-dev","path":"/Users/bryphe/oni2/test.oni-dev","scheme":["file"]},[{"startLineNumber":4,"startColumn":5,"endLineNumber":4,"endColumn":11,"message":"diag 1","source":"","code":"","severity":8,"relatedInformation":[]}]]]]
|};

let jsonListToList = json =>
  switch (json) {
  | `List(args) => args
  | _ => failwith("Expected list")
  };

describe("ExtHostProtocol", ({describe, _}) => {
  describe("IncomingNotifications", ({describe, _}) => {
    IncomingNotifications.(
      describe("Diagnostics", ({test, _}) => {
        Diagnostics.(
          DiagnosticsCollection.(
            test("parse collection", ({expect, _}) => {
              let diagnostics: option(DiagnosticsCollection.t) =
                parseChangeMany(
                  Yojson.Safe.from_string(diagnosticCollectionJSON)
                  |> jsonListToList,
                );
              switch (diagnostics) {
              | None => failwith("Expected successful parse")
              | Some(v) =>
                expect.string(v.name).toEqual("test");
                expect.int(List.length(v.perFileDiagnostics)).toBe(1);

                let (uri, diags) = List.hd(v.perFileDiagnostics);
                expect.string(Uri.toString(uri)).toEqual(
                  "file:///Users/bryphe/oni2/test.oni-dev",
                );
                expect.int(List.length(diags)).toBe(1);
              };
            })
          )
        )
      })
    )
  })
});
