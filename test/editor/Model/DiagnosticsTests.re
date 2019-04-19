open TestFramework;

module Buffer = Oni_Model.Buffer;
module Diagnostics = Oni_Model.Diagnostics;

describe("Diagnostics", ({describe, _}) =>
  describe("change", ({test, _}) => {
    test("simple diagnostic add", ({expect}) => {
        let _buffer = Buffer.ofLines([||]);
        let _diagnostics = Diagnostics.create();
        expect.bool(true).toBe(false);
    });
  })
);
