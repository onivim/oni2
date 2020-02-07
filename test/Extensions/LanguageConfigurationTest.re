open EditorCoreTypes;
open Oni_Core;
open Oni_Core_Test;
open Oni_Extensions;

open TestFramework;

open ExtHostProtocol;

let json = Yojson.Safe.from_string;

describe("LanguageConfiguration", ({describe, _}) => {
  open LanguageConfiguration;
  describe("AutoClosingPair", ({test, _}) => {
    open AutoClosingPair;
    test("decode tuple", ({expect, _}) => {
      let result = json({|["a", "b"]|})
      |> Json.Decode.decode_value(decode);
      //let result = decode("erroneous");

      switch(result) {
      | Ok({openPair, closePair, _}) =>
          expect.string(openPair).toEqual("a");
          expect.string(closePair).toEqual("b");
      | _ => failwith("parse failed");
      }
    });
    test("decode simple object", ({expect, _}) => {
      let result = json({|{"open": "c", "close": "d"}|})
      |> Json.Decode.decode_value(decode);
      //let result = decode("erroneous");

      switch(result) {
      | Ok({openPair, closePair, _}) =>
          expect.string(openPair).toEqual("c");
          expect.string(closePair).toEqual("d");
      | _ => failwith("parse failed");
      }
    });
  });
});
