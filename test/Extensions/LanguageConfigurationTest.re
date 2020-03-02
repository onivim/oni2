open Oni_Core;
open Oni_Core_Test;
open Oni_Extensions;

open TestFramework;

let json = Yojson.Safe.from_string;

describe("LanguageConfiguration", ({describe, test, _}) => {
  describe("AutoClosingPair", ({test, _}) => {
    open LanguageConfiguration.AutoClosingPair;
    test("decode tuple", ({expect, _}) => {
      let result = json({|["a", "b"]|}) |> Json.Decode.decode_value(decode);

      switch (result) {
      | Ok({openPair, closePair, _}) =>
        expect.string(openPair).toEqual("a");
        expect.string(closePair).toEqual("b");
      | _ => failwith("parse failed")
      };
    });
    test("decode simple object", ({expect, _}) => {
      let result =
        json({|{"open": "c", "close": "d"}|})
        |> Json.Decode.decode_value(decode);

      switch (result) {
      | Ok({openPair, closePair, notIn}) =>
        expect.string(openPair).toEqual("c");
        expect.string(closePair).toEqual("d");
        expect.equal(notIn, []);
      | _ => failwith("parse failed")
      };
    });
    test("decode object with notIn list", ({expect, _}) => {
      let result =
        json({|{"open": "c", "close": "d", "notIn": ["string"]}|})
        |> Json.Decode.decode_value(decode);

      switch (result) {
      | Ok({openPair, closePair, notIn}) =>
        expect.string(openPair).toEqual("c");
        expect.string(closePair).toEqual("d");
        expect.equal(notIn, [String]);
      | _ => failwith("parse failed")
      };
    });
    test("decode object with notIn string", ({expect, _}) => {
      let result =
        json({|{"open": "c", "close": "d", "notIn": "comment"}|})
        |> Json.Decode.decode_value(decode);

      switch (result) {
      | Ok({openPair, closePair, notIn}) =>
        expect.string(openPair).toEqual("c");
        expect.string(closePair).toEqual("d");
        expect.equal(notIn, [Comment]);
      | _ => failwith("parse failed")
      };
    });
  });

  test("decode autoCloseBefore", ({expect, _}) => {
    let result =
      json({|{"autoCloseBefore": "abc"}|})
      |> Json.Decode.decode_value(LanguageConfiguration.decode);

    switch (result) {
    | Ok(({autoCloseBefore, _}: LanguageConfiguration.t)) =>
      expect.equal(autoCloseBefore, ["a", "b", "c"])
    | _ => failwith("parse failed")
    };
  });
});
