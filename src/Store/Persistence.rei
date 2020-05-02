open Oni_Core;
open Oni_Model;

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

  let instantiate: (string, unit => list(entry('state))) => t('state);

  let persist: t('state) => unit;
  let persistIfDirty: (t('state), 'state) => unit;

  let get: (Schema.item('state, 'value), t('state)) => 'value;
};

// BUILTIN STORES

module Global: {
  let version: Schema.item(State.t, string);
  let workspace: Schema.item(State.t, option(string));

  let store: Store.t(State.t);
};

module Workspace: {
  type state = (State.t, Revery.Window.t);

  let windowX: Schema.item(state, option(int));
  let windowY: Schema.item(state, option(int));
  let windowWidth: Schema.item(state, int);
  let windowHeight: Schema.item(state, int);
  let windowMaximized: Schema.item(state, bool);

  let storeFor: string => Store.t(state);
};
