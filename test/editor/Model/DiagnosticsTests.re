open TestFramework;

open Oni_Core.Types;

module Buffer = Oni_Model.Buffer;
module Diagnostics = Oni_Model.Diagnostics;

let singleDiagnostic = [
    Diagnostics.Diagnostic.create(~range=Range.zero, ()),
]

describe("Diagnostics", ({describe, _}) =>
  describe("change", ({test, _}) =>
    test("simple diagnostic add", ({expect}) => {
      let buffer = Buffer.ofLines([||]);
      let v = Diagnostics.create();

      let v = Diagnostics.change(v, buffer, "test_key1", singleDiagnostic);

      let diagnostics = Diagnostics.getDiagnostics(v, buffer);

      expect.int(List.length(diagnostics)).toBe(1);
    })
  )
);
