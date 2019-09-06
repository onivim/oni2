open TestFramework;

open Oni_Core;
open Oni_Core.Types;

module Buffer = Oni_Model.Buffer;
module Diagnostics = Oni_Model.Diagnostics;

let singleDiagnostic = [
  Diagnostics.Diagnostic.create(~range=Range.zero, ~message="single error", ()),
];

let doubleDiagnostic = [
  Diagnostics.Diagnostic.create(~range=Range.zero, ~message="error 1", ()),
  Diagnostics.Diagnostic.create(~range=Range.zero, ~message="error 2", ()),
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

  describe("getDiagnosticsAtPosition", ({test, _}) =>
    test("simple diagnostic", ({expect}) => {

    let singleDiagnostic = [
    Diagnostics.Diagnostic.create(~range=Range.ofInt0(
    ~startLine = 1,
    ~startCharacter = 1,
    ~endLine=2,
    ~endCharacter=2,
    ()
    ), ~message="single error", ()),
  ];
      
      let buffer = Buffer.ofLines([||]);
      let v = Diagnostics.create();
      
      let v = Diagnostics.change(v, buffer, "test_key1", singleDiagnostic);

      let diags = Diagnostics.getDiagnosticsAtPosition(v, buffer, Position.ofInt0(0, 0));
      expect.int(List.length(diags)).toBe(0);
      
      let diags = Diagnostics.getDiagnosticsAtPosition(v, buffer, Position.ofInt0(1, 1));
      expect.int(List.length(diags)).toBe(1);
      
      let diags = Diagnostics.getDiagnosticsAtPosition(v, buffer, Position.ofInt0(2, 1));
      expect.int(List.length(diags)).toBe(1);
      
      let diags = Diagnostics.getDiagnosticsAtPosition(v, buffer, Position.ofInt0(2, 2));
      expect.int(List.length(diags)).toBe(1);
      
      let diags = Diagnostics.getDiagnosticsAtPosition(v, buffer, Position.ofInt0(2, 3));
      expect.int(List.length(diags)).toBe(0);
      
      let diags = Diagnostics.getDiagnosticsAtPosition(v, buffer, Position.ofInt0(3, 0));
      expect.int(List.length(diags)).toBe(0);
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
