open Oni_Core;

type codec('state, 'value) = {
  equal: ('value, 'value) => bool,
  encode: 'value => Json.t,
  decode: Json.decoder('value),
};

let custom = (~equal, ~encode, ~decode) => {equal, encode, decode};

let int = {
  equal: Int.equal,
  encode: Json.Encode.int,
  decode: Json.Decode.int,
};
let string = {
  equal: String.equal,
  encode: Json.Encode.string,
  decode: Json.Decode.string,
};
let option = codec => {
  equal: Option.equal(codec.equal),
  encode: Json.Encode.option(codec.encode),
  decode: Json.Decode.maybe(codec.decode),
};

type definition('state, 'value) = {
  key: string,
  default: 'value,
  codec: codec('state, 'value),
  get: 'state => 'value,
};

let define = (key, codec, default, get) => {key, codec, default, get};

