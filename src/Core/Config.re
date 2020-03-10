open Revery;

module Log = (val Log.withNamespace("Oni2.Core.Config"));
module Lookup = Kernel.KeyedStringMap;

// CONFIGURATION

type t = Lookup.t(Json.t);

let empty = Lookup.empty;

let fromList = entries =>
  entries
  |> List.to_seq
  |> Seq.map(((keyName, entry)) => (Lookup.key(keyName), entry))
  |> Lookup.of_seq;

let fromFile = path => {
  switch (Yojson.Safe.from_file(path)) {
  | `Assoc(items) => fromList(items)

  | _ =>
    Log.errorf(m =>
      m("Expected configuration file to contain a JSON object")
    );
    empty;

  | exception (Yojson.Json_error(message)) =>
    Log.errorf(m =>
      m("Error encoutnered reading configuration file: %s", message)
    );
    empty;
  };
};

let union = (xs, ys) =>
  Lookup.union(
    (key: Lookup.key, _x, y) => {
      Log.warnf(m =>
        m("Encountered duplicate key: %s", Lookup.keyName(key))
      );
      Some(y);
    },
    xs,
    ys,
  );
let unionMany = lookups => List.fold_left(union, Lookup.empty, lookups);

module Schema = {
  type codec('a) = {
    decode: Json.decoder('a),
    encode: Json.encoder('a),
  };
  type spec = {
    key: Lookup.key,
    default: Json.t,
  };
  type setting('a) = {
    spec,
    get: t => 'a,
  };

  let bool = {decode: Json.Decode.bool, encode: Json.Encode.bool};
  let int = {decode: Json.Decode.int, encode: Json.Encode.int};
  let string = {decode: Json.Decode.string, encode: Json.Encode.string};
  let list = valueCodec => {
    decode: Json.Decode.list(valueCodec.decode),
    encode: Json.Encode.list(valueCodec.encode),
  };

  let custom = (~decode, ~encode) => {decode, encode};

  let setting = (keyName, codec, ~default) => {
    let key = Lookup.key(keyName);

    {
      spec: {
        key,
        default: codec.encode(default),
      },
      get: lookup => {
        switch (Lookup.find_opt(key, lookup)) {
        | Some(jsonValue) =>
          switch (Json.Decode.decode_value(codec.decode, jsonValue)) {
          | Ok(value) => value
          | Error(err) =>
            Log.errorf(m =>
              m(
                "Error decoding configuration value `%s`:\n\t%s",
                keyName,
                Json.Decode.string_of_error(err),
              )
            );
            default;
          }
        | None => default
        };
      },
    };
  };
};
