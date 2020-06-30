open Revery;

module Log = (val Log.withNamespace("Oni2.Core.Config"));
module Lookup = Kernel.KeyedStringTree;

type key = Lookup.path;
type resolver = key => option(Json.t);

let key = Lookup.path;
let keyAsString = Lookup.key;

// SETTINGS

module Settings = {
  type t = Lookup.t(Json.t);

  let empty = Lookup.empty;

  let fromList = entries =>
    entries
    |> List.map(((key, entry)) => (Lookup.path(key), entry))
    |> Lookup.fromList;

  let fromJson = json => {
    switch (json) {
    | `Assoc(items) => fromList(items)

    | _ =>
      Log.errorf(m => m("Expected file to contain a JSON object"));
      empty;
    };
  };

  let fromFile = path =>
    try(path |> Yojson.Safe.from_file |> fromJson) {
    | Yojson.Json_error(message) =>
      Log.errorf(m => m("Failed to read file %s: %s", path, message));
      empty;
    };

  let get = Lookup.get;

  let union = (xs, ys) =>
    Lookup.union(
      (path, _x, y) => {
        Log.warnf(m => m("Encountered duplicate key: %s", Lookup.key(path)));
        Some(y);
      },
      xs,
      ys,
    );
  let unionMany = lookups => List.fold_left(union, Lookup.empty, lookups);

  let diff = (xs, ys) =>
    Lookup.merge(
      (_path, x, y) =>
        switch (x, y) {
        | (Some(x), Some(y)) when x == y => None
        | (Some(_), Some(y)) => Some(y)
        | (Some(_), None) => Some(Json.Encode.null)
        | (None, Some(value)) => Some(value)
        | (None, None) => failwith("unreachable")
        },
      xs,
      ys,
    );

  let changed = (xs, ys) =>
    diff(xs, ys) |> Lookup.map(_ => Json.Encode.bool(true));

  let keys = settings =>
    Lookup.fold((key, _, acc) => [key, ...acc], settings, []);

  let rec toJson = node =>
    switch ((node: t)) {
    | Node(children) =>
      Json.Encode.obj(
        children
        |> Lookup.KeyedMap.to_seq
        |> Seq.map(((key, value)) => (key, toJson(value)))
        |> List.of_seq,
      )
    | Leaf(value) => value
    };
};

// SCHEMA

module Schema = {
  type spec = {
    path: Lookup.path,
    default: Json.t,
  };
  type t = Lookup.t(spec);

  let fromList = specs =>
    specs |> List.map(spec => (spec.path, spec)) |> Lookup.fromList;

  let union = (xs, ys) =>
    Lookup.union(
      (path, _x, y) => {
        Log.warnf(m => m("Encountered duplicate key: %s", Lookup.key(path)));
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
      get: resolver => 'a,
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

    let setting = (key, codec, ~default) => {
      let path = Lookup.path(key);

      {
        spec: {
          path,
          default: codec.encode(default),
        },
        get: resolve => {
          switch (resolve(path)) {
          | Some(jsonValue) =>
            switch (Json.Decode.decode_value(codec.decode, jsonValue)) {
            | Ok(value) => value
            | Error(err) =>
              Log.errorf(m =>
                m(
                  "Failed to decode value for `%s`:\n\t%s",
                  key,
                  Json.Decode.string_of_error(err),
                )
              );
              default;
            }
          | None =>
            Log.warnf(m => m("Missing default value for `%s`", key));
            default;
          };
        },
      };
    };
  };

  include DSL;
};

module Sub = {
  module type Config = {
    type configValue;
    let schema: Schema.setting(configValue);
    type msg;
  };
  module type S = {
    type configValue;
    type msg;

    let create:
      (~config: resolver, ~name: string, ~toMsg: configValue => msg) =>
      Isolinear.Sub.t(msg);
  };
  module Make = (Config: Config) => {
    type configValue = Config.configValue;
    type msg = Config.msg;
    type params = {
      config: resolver,
      name: string,
      toMsg: Config.configValue => Config.msg,
    };
    module InnerSub =
      Isolinear.Sub.Make({
        type nonrec msg = Config.msg;
        type nonrec params = params;
        type state = {lastValue: Config.configValue};

        let name = "Config.Subscription";
        let id = params => {
          params.name;
        };

        let init = (~params, ~dispatch) => {
          let lastValue = Config.schema.get(params.config);
          dispatch(params.toMsg(lastValue));
          {lastValue: lastValue};
        };

        let update = (~params, ~state, ~dispatch) => {
          let newValue = Config.schema.get(params.config);
          if (newValue != state.lastValue) {
            dispatch(params.toMsg(newValue));
          };

          {lastValue: newValue};
        };

        let dispose = (~params as _, ~state as _) => {
          ();
        };
      });

    let create = (~config, ~name, ~toMsg) => {
      InnerSub.create({config, name, toMsg});
    };
  };
};
