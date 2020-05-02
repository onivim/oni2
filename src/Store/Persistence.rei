open Oni_Model;
open Oni_Core.Persistence;

module Global: {
  let version: Schema.item(State.t, string);
  let workspace: Schema.item(State.t, option(string));

  let persist: State.t => unit;

  let get: Schema.item(State.t, 'value) => 'value;
};

module Workspace: {
  type state = (State.t, Revery.Window.t);

  let windowX: Schema.item(state, option(int));
  let windowY: Schema.item(state, option(int));
  let windowWidth: Schema.item(state, int);
  let windowHeight: Schema.item(state, int);
  let windowMaximized: Schema.item(state, bool);

  let storeFor: string => Store.t(state);

  let persist: ('state, Store.t('state)) => unit;

  let get: (Schema.item('state, 'value), Store.t('state)) => 'value;
};
