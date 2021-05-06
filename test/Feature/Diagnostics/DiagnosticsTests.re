open EditorCoreTypes;
open Oni_Core;
open TestFramework;
open Feature_Diagnostics;
module Diagnostics = Feature_Diagnostics;

module LineNumber = EditorCoreTypes.LineNumber;

let zeroRange =
  CharacterRange.{
    start: CharacterPosition.zero,
    stop: CharacterPosition.zero,
  };

let singleDiagnostic = [
  Diagnostic.create(
    ~range=zeroRange,
    ~message="single error",
    ~severity=Exthost.Diagnostic.Severity.Error,
    ~tags=[],
  ),
];

let doubleDiagnostic = [
  Diagnostic.create(
    ~range=zeroRange,
    ~message="error 1",
    ~severity=Exthost.Diagnostic.Severity.Error,
    ~tags=[],
  ),
  Diagnostic.create(
    ~range=zeroRange,
    ~message="error 2",
    ~severity=Exthost.Diagnostic.Severity.Error,
    ~tags=[],
  ),
];

let buffer = Buffer.ofLines(~font=Font.default(), ~id=0, [||]);
let uri = Buffer.getUri(buffer);

let buffer1 = Buffer.ofLines(~font=Font.default(), ~id=1, [||]);
let uri1 = Buffer.getUri(buffer1);

let buffer2 = Buffer.ofLines(~font=Font.default(), ~id=2, [||]);
let uri2 = Buffer.getUri(buffer2);

describe("Diagnostics", ({describe, _}) => {
  describe("count", ({test, _}) => {
    test("change single buffer, multiple keys", ({expect, _}) => {
      let v = Diagnostics.initial;
      let v =
        Diagnostics.Testing.change(v, uri, "test_key1", singleDiagnostic);

      expect.int(Diagnostics.count(v)).toBe(1);

      let v =
        Diagnostics.Testing.change(v, uri, "test_key2", doubleDiagnostic);
      expect.int(Diagnostics.count(v)).toBe(3);

      let v = Diagnostics.Testing.clear(v, "test_key1");
      expect.int(Diagnostics.count(v)).toBe(2);
    });

    test("change multiple buffers, multiple keys", ({expect, _}) => {
      let v = Diagnostics.initial;

      let v =
        Diagnostics.Testing.change(v, uri1, "test_key1", singleDiagnostic);
      let v =
        Diagnostics.Testing.change(v, uri2, "test_key1", doubleDiagnostic);

      expect.int(Diagnostics.count(v)).toBe(3);

      let v = Diagnostics.Testing.clear(v, "test_key1");
      expect.int(Diagnostics.count(v)).toBe(0);
    });
  });

  describe("getDiagnostics", ({test, _}) =>
    test("no diagnostics", ({expect, _}) => {
      let buffer = Buffer.ofLines(~font=Font.default(), [||]);
      let v = Diagnostics.initial;

      let diagnostics = Diagnostics.getDiagnostics(v, buffer);

      expect.int(List.length(diagnostics)).toBe(0);
    })
  );

  describe("getDiagnosticsAtPosition", ({test, _}) =>
    test("simple diagnostic", ({expect, _}) => {
      let singleDiagnostic = [
        Diagnostic.create(
          ~range=
            CharacterRange.{
              start:
                CharacterPosition.{
                  line: LineNumber.(zero + 1),
                  character: CharacterIndex.(zero + 1),
                },
              stop:
                CharacterPosition.{
                  line: LineNumber.(zero + 2),
                  character: CharacterIndex.(zero + 2),
                },
            },
          ~message="single error",
          ~severity=Exthost.Diagnostic.Severity.Error,
          ~tags=[],
        ),
      ];

      let v = Diagnostics.initial;

      let v =
        Diagnostics.Testing.change(v, uri, "test_key1", singleDiagnostic);

      let diags =
        Diagnostics.getDiagnosticsAtPosition(
          v,
          buffer,
          CharacterPosition.{
            line: LineNumber.zero,
            character: CharacterIndex.zero,
          },
        );
      expect.int(List.length(diags)).toBe(0);

      let diags =
        Diagnostics.getDiagnosticsAtPosition(
          v,
          buffer,
          CharacterPosition.{
            line: LineNumber.(zero + 1),
            character: CharacterIndex.(zero + 1),
          },
        );
      expect.int(List.length(diags)).toBe(1);

      let diags =
        Diagnostics.getDiagnosticsAtPosition(
          v,
          buffer,
          CharacterPosition.{
            line: LineNumber.(zero + 2),
            character: CharacterIndex.(zero + 1),
          },
        );
      expect.int(List.length(diags)).toBe(1);

      let diags =
        Diagnostics.getDiagnosticsAtPosition(
          v,
          buffer,
          CharacterPosition.{
            line: LineNumber.(zero + 2),
            character: CharacterIndex.(zero + 2),
          },
        );
      expect.int(List.length(diags)).toBe(1);

      let diags =
        Diagnostics.getDiagnosticsAtPosition(
          v,
          buffer,
          CharacterPosition.{
            line: LineNumber.(zero + 2),
            character: CharacterIndex.(zero + 3),
          },
        );
      expect.int(List.length(diags)).toBe(0);

      let diags =
        Diagnostics.getDiagnosticsAtPosition(
          v,
          buffer,
          CharacterPosition.{
            line: LineNumber.(zero + 3),
            character: CharacterIndex.zero,
          },
        );
      expect.int(List.length(diags)).toBe(0);
    })
  );
  describe("clear", ({test, _}) => {
    test("single diagnostic", ({expect, _}) => {
      let buffer = Buffer.ofLines(~font=Font.default(), [||]);

      let v = Diagnostics.initial;
      let v =
        Diagnostics.Testing.change(v, uri, "test_key1", singleDiagnostic);
      let v = Diagnostics.Testing.clear(v, "test_key1");

      let diagnostics = Diagnostics.getDiagnostics(v, buffer);

      expect.int(List.length(diagnostics)).toBe(0);
    });
    test("doesn't remove other keys", ({expect, _}) => {
      let buffer = Buffer.ofLines(~font=Font.default(), [||]);

      let v = Diagnostics.initial;
      let v =
        Diagnostics.Testing.change(v, uri, "test_key1", singleDiagnostic);
      let v =
        Diagnostics.Testing.change(v, uri, "test_key2", doubleDiagnostic);
      let v = Diagnostics.Testing.clear(v, "test_key1");

      let diagnostics = Diagnostics.getDiagnostics(v, buffer);

      expect.int(List.length(diagnostics)).toBe(2);
    });
  });

  describe("change", ({test, _}) => {
    test("simple diagnostic add", ({expect, _}) => {
      let buffer = Buffer.ofLines(~font=Font.default(), [||]);
      let v = Diagnostics.initial;

      let v =
        Diagnostics.Testing.change(v, uri, "test_key1", singleDiagnostic);

      let diagnostics = Diagnostics.getDiagnostics(v, buffer);

      expect.int(List.length(diagnostics)).toBe(1);
    });

    test("diagnostics from multiple sources", ({expect, _}) => {
      let buffer = Buffer.ofLines(~font=Font.default(), [||]);
      let v = Diagnostics.initial;

      let v =
        Diagnostics.Testing.change(v, uri, "test_key1", singleDiagnostic);
      let v =
        Diagnostics.Testing.change(v, uri, "test_key2", doubleDiagnostic);

      let diagnostics = Diagnostics.getDiagnostics(v, buffer);

      expect.int(List.length(diagnostics)).toBe(3);
    });

    test(
      "clearing diagnostic from one source doesn't clear the other source",
      ({expect, _}) => {
      let buffer = Buffer.ofLines(~font=Font.default(), [||]);
      let v = Diagnostics.initial;

      let v =
        Diagnostics.Testing.change(v, uri, "test_key1", singleDiagnostic);
      let v =
        Diagnostics.Testing.change(v, uri, "test_key2", doubleDiagnostic);
      let v = Diagnostics.Testing.change(v, uri, "test_key1", []);

      let diagnostics = Diagnostics.getDiagnostics(v, buffer);

      expect.int(List.length(diagnostics)).toBe(2);
    });
  });
});
