open Oni_Model;
open Oni_Core.Persistence;

module Global: {
  let version: unit => string;
  let workspace: unit => option(string);

  let persist: State.t => unit;
};

module Workspace: {
  type state = (State.t, Revery.Window.t);

  let storeFor: string => Store.t(state);

  let windowX: Store.t(state) => option(int);
  let windowY: Store.t(state) => option(int);
  let windowWidth: Store.t(state) => int;
  let windowHeight: Store.t(state) => int;
  let windowMaximized: Store.t(state) => bool;

  let persist: (state, Store.t(state)) => unit;
};
