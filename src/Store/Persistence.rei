open Oni_Core;
open Oni_Model;

type codec('value);

let custom:
  (
    ~equal: ('value, 'value) => bool,
    ~encode: Json.encoder('value),
    ~decode: Json.decoder('value)
  ) =>
  codec('value);
let int: codec(int);
let string: codec(Stdlib.String.t);
let option: codec('value) => codec(option('value));

type definition('state, 'value);
let define:
  (string, codec('value), 'value, 'state => 'value) =>
  definition('state, 'value);

type entry('state);
let entry: definition('state, _) => entry('state);

type store('state);
let instantiate: (string, unit => list(entry('state))) => store('state);
let persist: store('state) => unit;
let persistIfDirty: (store('state), 'state) => unit;

let get: (definition('state, 'value), store('state)) => 'value;

module Global: {
  let version: definition(State.t, string);
  let workspace: definition(State.t, option(string));

  let store: store(State.t);
};

module Workspace: {
  type state = (State.t, Revery.Window.t);
  let windowX: definition(state, int);
  let windowY: definition(state, int);
  let windowWidth: definition(state, int);
  let windowHeight: definition(state, int);

  let storeFor: string => store(state);
};
