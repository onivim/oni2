// API reference: https://mattjbray.github.io/ocaml-decoders/decoders/decoders-yojson/Decoders_yojson/index.html

type t = Yojson.Safe.json;

module Decode = {
  include Decoders_yojson.Safe.Decode;

  let default = default => map(Utility.Option.value(~default));
};

module Encode = {
  include Decoders_yojson.Safe.Encode;
};

type decoder('a) = Decode.decoder('a);
type encoder('a) = Encode.encoder('a);
