open EditorCoreTypes;
open Oni_Core;
open TestFramework;

module Diagnostic = Oni_Model.Diagnostic;
module Diagnostics = Oni_Model.Diagnostics;

let zeroRange =
  Range.{
    start: Location.{line: Index.zero, column: Index.zero},
    stop: Location.{line: Index.zero, column: Index.zero},
  };

let singleDiagnostic = [
  Diagnostic.create(~range=zeroRange, ~message="single error", ()),
];

let doubleDiagnostic = [
  Diagnostic.create(~range=zeroRange, ~message="error 1", ()),
  Diagnostic.create(~range=zeroRange, ~message="error 2", ()),
];

let buffer = Buffer.ofLines(~id=0, [||]);
let uri = Buffer.getUri(buffer);

let buffer1 = Buffer.ofLines(~id=1, [||]);
let uri1 = Buffer.getUri(buffer1);

let buffer2 = Buffer.ofLines(~id=2, [||]);
let uri2 = Buffer.getUri(buffer2);

describe("Diagnostics", ({describe, _}) => {
  describe("count", ({test, _}) => {
    test("change single buffer, multiple keys", ({expect, _}) => {
      let v = Diagnostics.create();
      let v = Diagnostics.change(v, uri, "test_key1", singleDiagnostic);

      expect.int(Diagnostics.count(v)).toBe(1);

      let v = Diagnostics.change(v, uri, "test_key2", doubleDiagnostic);
      expect.int(Diagnostics.count(v)).toBe(3);

      let v = Diagnostics.clear(v, "test_key1");
      expect.int(Diagnostics.count(v)).toBe(2);
    });

    test("change multiple buffers, multiple keys", ({expect, _}) => {
      let v = Diagnostics.create();

      let v = Diagnostics.change(v, uri1, "test_key1", singleDiagnostic);
      let v = Diagnostics.change(v, uri2, "test_key1", doubleDiagnostic);

      expect.int(Diagnostics.count(v)).toBe(3);

      let v = Diagnostics.clear(v, "test_key1");
      expect.int(Diagnostics.count(v)).toBe(0);
    });
  });

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
        Diagnostic.create(
          ~range=
            Range.{
              start:
                Location.{line: Index.(zero + 1), column: Index.(zero + 1)},
              stop:
                Location.{line: Index.(zero + 2), column: Index.(zero + 2)},
            },
          ~message="single error",
          (),
        ),
      ];

      let v = Diagnostics.create();

      let v = Diagnostics.change(v, uri, "test_key1", singleDiagnostic);

      let diags =
        Diagnostics.getDiagnosticsAtPosition(
          v,
          buffer,
          Location.{line: Index.zero, column: Index.zero},
        );
      expect.int(List.length(diags)).toBe(0);

      let diags =
        Diagnostics.getDiagnosticsAtPosition(
          v,
          buffer,
          Location.{line: Index.(zero + 1), column: Index.(zero + 1)},
        );
      expect.int(List.length(diags)).toBe(1);

      let diags =
        Diagnostics.getDiagnosticsAtPosition(
          v,
          buffer,
          Location.{line: Index.(zero + 2), column: Index.(zero + 1)},
        );
      expect.int(List.length(diags)).toBe(1);

      let diags =
        Diagnostics.getDiagnosticsAtPosition(
          v,
          buffer,
          Location.{line: Index.(zero + 2), column: Index.(zero + 2)},
        );
      expect.int(List.length(diags)).toBe(1);

      let diags =
        Diagnostics.getDiagnosticsAtPosition(
          v,
          buffer,
          Location.{line: Index.(zero + 2), column: Index.(zero + 3)},
        );
      expect.int(List.length(diags)).toBe(0);

      let diags =
        Diagnostics.getDiagnosticsAtPosition(
          v,
          buffer,
          Location.{line: Index.(zero + 3), column: Index.zero},
        );
      expect.int(List.length(diags)).toBe(0);
    })
  );
  describe("clear", ({test, _}) => {
    test("single diagnostic", ({expect}) => {
      let buffer = Buffer.ofLines([||]);

      let v = Diagnostics.create();
      let v = Diagnostics.change(v, uri, "test_key1", singleDiagnostic);
      let v = Diagnostics.clear(v, "test_key1");

      let diagnostics = Diagnostics.getDiagnostics(v, buffer);

      expect.int(List.length(diagnostics)).toBe(0);
    });
    test("doesn't remove other keys", ({expect}) => {
      let buffer = Buffer.ofLines([||]);

      let v = Diagnostics.create();
      let v = Diagnostics.change(v, uri, "test_key1", singleDiagnostic);
      let v = Diagnostics.change(v, uri, "test_key2", doubleDiagnostic);
      let v = Diagnostics.clear(v, "test_key1");

      let diagnostics = Diagnostics.getDiagnostics(v, buffer);

      expect.int(List.length(diagnostics)).toBe(2);
    });
  });

  describe("change", ({test, _}) => {
    test("simple diagnostic add", ({expect}) => {
      let buffer = Buffer.ofLines([||]);
      let v = Diagnostics.create();

      let v = Diagnostics.change(v, uri, "test_key1", singleDiagnostic);

      let diagnostics = Diagnostics.getDiagnostics(v, buffer);

      expect.int(List.length(diagnostics)).toBe(1);
    });

    test("diagnostics from multiple sources", ({expect}) => {
      let buffer = Buffer.ofLines([||]);
      let v = Diagnostics.create();

      let v = Diagnostics.change(v, uri, "test_key1", singleDiagnostic);
      let v = Diagnostics.change(v, uri, "test_key2", doubleDiagnostic);

      let diagnostics = Diagnostics.getDiagnostics(v, buffer);

      expect.int(List.length(diagnostics)).toBe(3);
    });

    test(
      "clearing diagnostic from one source doesn't clear the other source",
      ({expect}) => {
      let buffer = Buffer.ofLines([||]);
      let v = Diagnostics.create();

      let v = Diagnostics.change(v, uri, "test_key1", singleDiagnostic);
      let v = Diagnostics.change(v, uri, "test_key2", doubleDiagnostic);
      let v = Diagnostics.change(v, uri, "test_key1", []);

      let diagnostics = Diagnostics.getDiagnostics(v, buffer);

      expect.int(List.length(diagnostics)).toBe(2);
    });
  });
});
