open EditorCoreTypes;
open Oni_Core;
open TestFramework;
open Feature_LanguageSupport;

describe("Diagnostics", ({describe, _}) => {
  describe("explode", ({test, _}) => {
    test("Regression Test for #1607 - stack overflow", ({expect, _}) => {
      let start = Location.{line: Index.zero, column: Index.zero};
      let stop =
        Location.{line: Index.(zero + 424242), column: Index.(zero + 424242)};

      let range = Range.{start, stop};

      let hugeDiagnostic =
        Diagnostic.create(~range, ~message="test diagnostic", ());
      let diags = Diagnostic.explode(Buffer.ofLines([||]), hugeDiagnostic);

      expect.int(List.length(diags)).toBe(1000);
    })
  })
});
