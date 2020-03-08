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
  type decoder('a) =
    | Optional(Json.decoder('a)): decoder(option('a))
    | WithDefault('a, Json.decoder('a)): decoder('a);
  type setting('a) = t => 'a;

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

  let setting: type a. (string, decoder(a), t) => a =
    (keyName, decoder) => {
      let key = Lookup.key(keyName);

      lookup => {
        switch (decoder) {
        | Optional(decoder) =>
          switch (Lookup.find_opt(key, lookup)) {
          | Some(jsonValue) =>
            switch (Json.Decode.decode_value(decoder, jsonValue)) {
            | Ok(value) => Some(value)
            | Error(err) =>
              Log.errorf(m =>
                m(
                  "Error decoding configuration value `%s`:\n\t%s",
                  keyName,
                  Json.Decode.string_of_error(err),
                )
              );
              None;
            }
          | None =>
            Log.warnf(m =>
              m("Unknown configuration key requested: %s", keyName)
            );
            None;
          }
        | WithDefault(default, decoder) =>
          switch (Lookup.find_opt(key, lookup)) {
          | Some(jsonValue) =>
            switch (Json.Decode.decode_value(decoder, jsonValue)) {
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
          }
        };
      };
    };
};
