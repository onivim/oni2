open TestFramework;

open Oni_Core.Types;

module Buffer = Oni_Model.Buffer;
module Diagnostics = Oni_Model.Diagnostics;

let singleDiagnostic = [
  Diagnostics.Diagnostic.create(~range=Range.zero, ()),
];

let doubleDiagnostic = [
  Diagnostics.Diagnostic.create(~range=Range.zero, ()),
  Diagnostics.Diagnostic.create(~range=Range.zero, ()),
];

describe("Diagnostics", ({describe, _}) => {
  describe("getDiagnostics", ({test, _}) =>
    test("no diagnostics", ({expect}) => {
      let buffer = Buffer.ofLines([||]);
      let v = Diagnostics.create();

      let diagnostics = Diagnostics.getDiagnostics(v, buffer);

      expect.int(List.length(diagnostics)).toBe(0);
    })
  );

  describe("change", ({test, _}) => {
    test("simple diagnostic add", ({expect}) => {
      let buffer = Buffer.ofLines([||]);
      let v = Diagnostics.create();

      let v = Diagnostics.change(v, buffer, "test_key1", singleDiagnostic);

      let diagnostics = Diagnostics.getDiagnostics(v, buffer);

      expect.int(List.length(diagnostics)).toBe(1);
    });

    test("diagnostics from multiple sources", ({expect}) => {
      let buffer = Buffer.ofLines([||]);
      let v = Diagnostics.create();

      let v = Diagnostics.change(v, buffer, "test_key1", singleDiagnostic);
      let v = Diagnostics.change(v, buffer, "test_key2", doubleDiagnostic);

      let diagnostics = Diagnostics.getDiagnostics(v, buffer);

      expect.int(List.length(diagnostics)).toBe(3);
    });

    test(
      "clearing diagnostic from one source doesn't clear the other source",
      ({expect}) => {
      let buffer = Buffer.ofLines([||]);
      let v = Diagnostics.create();

      let v = Diagnostics.change(v, buffer, "test_key1", singleDiagnostic);
      let v = Diagnostics.change(v, buffer, "test_key2", doubleDiagnostic);
      let v = Diagnostics.change(v, buffer, "test_key1", []);

      let diagnostics = Diagnostics.getDiagnostics(v, buffer);

      expect.int(List.length(diagnostics)).toBe(2);
    });
  });
});
