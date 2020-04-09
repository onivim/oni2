type key;
type resolver = key => option(Json.t);

let key: string => key;
let keyAsString: key => string;

// SETTINGS

module Settings: {
  type t;

  let empty: t;

  let fromList: list((string, Json.t)) => t;
  let fromJson: Json.t => t;
  let fromFile: string => t;

  let get: (key, t) => option(Json.t);

  let union: (t, t) => t;
  let unionMany: list(t) => t;

  /** Returns the set of changed keys with its new value, or `null` if removed */
  let diff: (t, t) => t;

  /** Returns the set of changed keys with the value `true`, intended for conversion to Json to mimic weird JavaScript semantics */
  let changed: (t, t) => t;

  let keys: t => list(key);

  let toJson: t => Json.t;
};

// SCHEMA

module Schema: {
  type t;
  type spec;

  let fromList: list(spec) => t;
  let union: (t, t) => t;
  let unionMany: list(t) => t;

  let defaults: t => Settings.t;

  type codec('a);
  type setting('a) = {
    spec,
    get: resolver => 'a,
  };

  // DSL

  module DSL: {
    let bool: codec(bool);
    let int: codec(int);
    let string: codec(string);
    let stringOpt: codec(option(string));
    let list: codec('a) => codec(list('a));

    let custom:
      (~decode: Json.decoder('a), ~encode: Json.encoder('a)) => codec('a);

    let setting: (string, codec('a), ~default: 'a) => setting('a);
  };

  let bool: codec(bool);
  let int: codec(int);
  let string: codec(string);
  let stringOpt: codec(option(string));
  let list: codec('a) => codec(list('a));

  let custom:
    (~decode: Json.decoder('a), ~encode: Json.encoder('a)) => codec('a);

  let setting: (string, codec('a), ~default: 'a) => setting('a);
};
