open Revery;

module Log = (val Log.withNamespace("Oni2.Core.Config"));
module Lookup = Kernel.KeyedStringMap;

// SETTINGS

module Settings = {
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
      Log.errorf(m => m("Expected file to contain a JSON object"));
      empty;

    | exception (Yojson.Json_error(message)) =>
      Log.errorf(m => m("Failed to read file %s: %s", path, message));
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
};

// SCHEMA

module Schema = {
  type spec = {
    key: Lookup.key,
    default: Json.t,
  };
  type t = Lookup.t(spec);

  let fromList = specs =>
    specs |> List.to_seq |> Seq.map(spec => (spec.key, spec)) |> Lookup.of_seq;

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

  let defaults = Lookup.map(spec => spec.default);

  // DSL

  module DSL = {
    type setting('a) = {
      spec,
      get: Settings.t => 'a,
    };
    type codec('a) = {
      decode: Json.decoder('a),
      encode: Json.encoder('a),
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
                  "Failed to decode value for `%s`:\n\t%s",
                  keyName,
                  Json.Decode.string_of_error(err),
                )
              );
              default;
            }
          | None =>
            Log.warnf(m => m("Missing default value for `%s`", keyName));
            default;
          };
        },
      };
    };
  };

  include DSL;
};
