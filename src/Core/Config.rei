type key;

// A pre-decode value from a configuration provider
type rawValue =
  | Json(Json.t)
  | Vim(VimSetting.t)
  | NotSet;

type resolver = (~vimSetting: option(string), key) => rawValue;
type fileTypeResolver = (~fileType: string) => resolver;

let emptyResolver: resolver;

let key: string => key;
let keyAsString: key => string;

// SETTINGS

module Settings: {
  type t;

  let empty: t;

  let fromList: list((string, Json.t)) => t;
  let fromJson: Json.t => t;
  let fromFile: FpExp.t(FpExp.absolute) => t;

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

  type vimSetting('a);

  // DSL

  module DSL: {
    let bool: codec(bool);
    let int: codec(int);
    let float: codec(float);
    let string: codec(string);
    let list: codec('a) => codec(list('a));
    let nullable: codec('a) => codec(option('a));

    let custom:
      (~decode: Json.decoder('a), ~encode: Json.encoder('a)) => codec('a);

    let vim: (string, VimSetting.t => 'a) => vimSetting('a);
    let vim2:
      (
        string,
        string,
        (option(VimSetting.t), option(VimSetting.t)) => option('a)
      ) =>
      vimSetting('a);

    let setting:
      (~vim: vimSetting('a)=?, string, codec('a), ~default: 'a) =>
      setting('a);
  };

  let bool: codec(bool);
  let int: codec(int);
  let float: codec(float);
  let string: codec(string);
  let list: codec('a) => codec(list('a));
  let nullable: codec('a) => codec(option('a));

  let vim: (string, VimSetting.t => 'a) => vimSetting('a);
  let vim2:
    (
      string,
      string,
      (option(VimSetting.t), option(VimSetting.t)) => option('a)
    ) =>
    vimSetting('a);

  let custom:
    (~decode: Json.decoder('a), ~encode: Json.encoder('a)) => codec('a);

  let setting:
    (~vim: vimSetting('a)=?, string, codec('a), ~default: 'a) => setting('a);
};

module Sub: {
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

  module Make:
    (Config: Config) =>
     S with type msg = Config.msg and type configValue = Config.configValue;
};
