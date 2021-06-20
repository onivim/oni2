open EditorCoreTypes;
open Oni_Core;
open TestFramework;
open Feature_Diagnostics;
module LineNumber = EditorCoreTypes.LineNumber;

describe("Diagnostics", ({describe, _}) => {
  describe("explode", ({test, _}) => {
    test("Regression Test for #1607 - stack overflow", ({expect, _}) => {
      let start =
        CharacterPosition.{
          line: LineNumber.zero,
          character: CharacterIndex.zero,
        };
      let stop =
        CharacterPosition.{
          line: LineNumber.(zero + 424242),
          character: CharacterIndex.(zero + 424242),
        };

      let range = CharacterRange.{start, stop};

      let hugeDiagnostic =
        Diagnostic.create(
          ~range,
          ~message="test diagnostic",
          ~severity=Exthost.Diagnostic.Severity.Error,
          ~tags=[],
        );
      let diags =
        Diagnostic.explode(
          Buffer.ofLines(~font=Font.default(), [||]),
          hugeDiagnostic,
        );

      expect.int(List.length(diags)).toBe(1000);
    })
  })
});
