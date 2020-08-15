// API reference: https://mattjbray.github.io/ocaml-decoders/decoders/decoders-yojson/Decoders_yojson/index.html

open EditorCoreTypes;

type t = Yojson.Safe.json;

module Decode: {
  include Decoders.Decode.S with type value = t;

  type objGetters = {
    field: getters(string),
    at: getters(list(string)),
    whatever: 'a. decoder('a) => 'a,
  }
  and getters('arg) = {
    optional: 'a. ('arg, decoder('a)) => option('a),
    required: 'a. ('arg, decoder('a)) => 'a,
    withDefault: 'a. ('arg, 'a, decoder('a)) => 'a,
    monadic: 'a. ('arg, decoder('a)) => decoder('a),
  };

  let obj: (objGetters => 'a) => decoder('a);
  let default: ('a, decoder(option('a))) => decoder('a);
};

module Encode: {include Decoders.Encode.S with type value = t;};

type decoder('a) = Decode.decoder('a);
type encoder('a) = Encode.encoder('a);

module Error: {
  type t = {
    range: Range.t,
    message: string,
  };

  let ofString: string => option(t);
};
