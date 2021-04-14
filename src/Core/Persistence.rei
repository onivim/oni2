// SCHEMA

module Schema: {
  module Codec: {
    type t('value);
    let custom:
      (
        ~equal: ('value, 'value) => bool,
        ~encode: Json.encoder('value),
        ~decode: Json.decoder('value)
      ) =>
      t('value);
  };

  let bool: Codec.t(bool);
  let int: Codec.t(int);
  let string: Codec.t(Stdlib.String.t);
  let option: Codec.t('value) => Codec.t(option('value));
  let value: Codec.t(Yojson.Safe.t);

  type item('state, 'value);
  let define:
    (string, Codec.t('value), 'value, 'state => 'value) =>
    item('state, 'value);
};

// STORE

module Store: {
  type t('state);

  type entry('state);
  let entry: Schema.item('state, _) => entry('state);

  let instantiate:
    (
      ~storeFolder: FpExp.t(FpExp.absolute)=?,
      string,
      unit => list(entry('state))
    ) =>
    t('state);

  let isDirty: ('state, t('state)) => bool;
  let update: ('state, t('state)) => unit;
  let write: t('state) => unit;
  let persist: ('state, t('state)) => unit;

  let get: (Schema.item('state, 'value), t('state)) => 'value;
};
