
open TestFramework;

open Exthost;

describe("Label", ({describe, _}) => {
  describe("parse", ({test, _}) => {
  
    test("raw string should pass through", ({expect, _}) => {
        let label = Label.of_string("text");
        expect.equal(label, [Label.Text("text")]);
    });
    test("extra quotes should be removed", ({expect, _}) => {
        let label = Label.of_string("\"text\"");
        expect.equal(label, [Label.Text("text")]);
    });
  });
});
