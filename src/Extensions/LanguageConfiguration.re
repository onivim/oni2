/*
 * LanguageConfiguration.re
 */
module Json = Oni_Core.Json;

module AutoClosingPair = {
  type scopes =
  | String
  | Comment;

  type t = {
    openPair: string,
    closePair: string,
    notIn: list(scopes),
  }

  module Decode = {
    open Json.Decode;

    // Pulled this from: https://github.com/mattjbray/ocaml-decoders/blob/550c05c16384c9f8d2ab6a36eebc8e1a2684a7e0/src/decode.mli#L106
    let (>>==::) = (fst, rest) => uncons(rest, fst);

    let tuple = 
      string
      >>==:: (openPair =>
        string >>==:: (closePair => succeed({
          openPair,
          closePair,
          notIn: [],
        })));

    let decode = {
      tuple;
    }
  
  };

  let decode = Decode.decode;
};

type t = {
  autoClosingPairs: list(AutoClosingPair.t)
}
