/*
 * LanguageConfiguration.re
 */
module Json = Oni_Core.Json;

module AutoClosingPair = {
  type scopes =
  | String
  | Comment
  | Other(string)

  type t = {
    openPair: string,
    closePair: string,
    notIn: list(scopes),
  }

  module Decode = {
    open Json.Decode;

    // Pulled this from: https://github.com/mattjbray/ocaml-decoders/blob/550c05c16384c9f8d2ab6a36eebc8e1a2684a7e0/src/decode.mli#L106
    let (>>==::) = (fst, rest) => uncons(rest, fst);

    let scope = string
    |> map(fun
    | "string" => String
    | "comment" => Comment
    | v => Other(v)
    );

    let tuple = 
      string
      >>==:: (openPair =>
        string >>==:: (closePair => succeed({
          openPair,
          closePair,
          notIn: [],
        })));

    let obj = obj(({field, _}) => {
       openPair: field.required("open", string),
       closePair: field.required("close", string),
       notIn: field.withDefault("notIn", [], list(scope)),
    });

    let decode = {
      one_of([
      ("tuple", tuple),
      ("object", obj)
      ]);
    }
  
  };

  let decode = Decode.decode;
};

type t = {
  autoClosingPairs: list(AutoClosingPair.t)
}
