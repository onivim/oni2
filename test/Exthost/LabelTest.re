
open TestFramework;

open Exthost;

describe("Label", ({describe, _}) => {
  describe("parse", ({test, _}) => {
  
    test("empty string", ({expect, _}) => {
        let label = Label.of_string("");
        expect.equal(label, []);
    });
    test("raw string should pass through", ({expect, _}) => {
        let label = Label.of_string("text");
        expect.equal(label, [Label.Text("text")]);
    });
    test("extra quotes should be removed", ({expect, _}) => {
        let label = Label.of_string("\"text\"");
        expect.equal(label, [Label.Text("text")]);
    });
    test("lone icon", ({expect, _}) => {
        let label = Label.of_string("\"$(alert)\"");
        expect.equal(label, [Label.Icon("alert")]);
    });
    test("text and icon", ({expect, _}) => {
        let label = Label.of_string("\"abc $(alert) def\"");
        expect.equal(label, [Label.Text("abc "), Label.Icon("alert"), Label.Text(" def")]);
    });
  });
});
