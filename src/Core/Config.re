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
  type decoder('a) = Json.decoder('a);
  type setting('a) = t => 'a;

  let bool = Json.Decode.bool;
  let int = Json.Decode.int;
  let string = Json.Decode.string;
  let list = Json.Decode.list;

  let custom = decoder => decoder;

  let setting = (keyName, decoder, ~default) => {
    let key = Lookup.key(keyName);

    lookup => {
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
      };
    };
  };
};
