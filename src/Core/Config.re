open Revery;

module Log = (val Log.withNamespace("Oni2.Core.Config"));

// INTERNAL

module Internal = {
  module Key: {
    type t =
      pri {
        hash: int,
        name: string,
      };
    let compare: (t, t) => int;
    let create: string => t;
  } = {
    type t = {
      hash: int,
      name: string,
    };

    let compare = (a, b) =>
      a.hash == b.hash ? compare(a.name, b.name) : compare(a.hash, b.hash);

    let create = name => {hash: Hashtbl.hash(name), name};
  };

  module Lookup = Map.Make(Key);
};

// CONFIGURATION

type key = Internal.Key.t;
let key = Internal.Key.create;

module Schema = {
  type decoder('a) =
    | Optional(Json.decoder('a)): decoder(option('a))
    | WithDefault('a, Json.decoder('a)): decoder('a);
  type setting('a) = {
    key,
    decoder: decoder('a),
  };

  let bool = Optional(Json.Decode.bool);
  let int = Optional(Json.Decode.int);
  let string = Optional(Json.Decode.string);
  // Reason apparently does not support refutation cases, so footgun it is!
  [@warning "-8"]
  let list = (Optional(valueDecoder)) =>
    Optional(Json.Decode.list(valueDecoder));

  // Reason apparently does not support refutation cases, so footgun it is!
  [@warning "-8"]
  let default = (defaultValue, Optional(decoder)) =>
    WithDefault(defaultValue, decoder);
  let custom = decoder => Optional(decoder);

  let setting = (name, decoder) => {key: key(name), decoder};
};

type t = Internal.Lookup.t(Json.t);

let empty = Internal.Lookup.empty;
let fromList = entries =>
  entries
  |> List.to_seq
  |> Seq.map(((keyName, entry)) => (Internal.Key.create(keyName), entry))
  |> Internal.Lookup.of_seq;
let union = (xs, ys) =>
  Internal.Lookup.union(
    (key, _x, y) => {
      Log.warnf(m => m("Encountered duplicate key: %s", key.name));
      Some(y);
    },
    xs,
    ys,
  );
let unionMany = lookups =>
  List.fold_left(union, Internal.Lookup.empty, lookups);

let get: type a. (Schema.setting(a), t) => a =
  (Schema.{key, decoder}, lookup) => {
    switch (decoder) {
    | Optional(decoder) =>
      switch (Internal.Lookup.find_opt(key, lookup)) {
      | Some(jsonValue) =>
        switch (Json.Decode.decode_value(decoder, jsonValue)) {
        | Ok(value) => Some(value)
        | Error(err) =>
          Log.errorf(m =>
            m(
              "Error decoding configuration value `%s`:\n\t%s",
              key.name,
              Json.Decode.string_of_error(err),
            )
          );
          None;
        }
      | None =>
        Log.warnf(m =>
          m("Unknown configuration key requested: %s", key.name)
        );
        None;
      }
    | WithDefault(default, decoder) =>
      switch (Internal.Lookup.find_opt(key, lookup)) {
      | Some(jsonValue) =>
        switch (Json.Decode.decode_value(decoder, jsonValue)) {
        | Ok(value) => value
        | Error(err) =>
          Log.errorf(m =>
            m(
              "Error decoding configuration value `%s`:\n\t%s",
              key.name,
              Json.Decode.string_of_error(err),
            )
          );
          default;
        }
      | None => default
      }
    };
  };
