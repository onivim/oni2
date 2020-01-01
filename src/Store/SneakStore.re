/*
 * SneakStoreConnector.re
 */

module Core = Oni_Core;
module Ext = Oni_Extensions;
module Model = Oni_Model;
module Actions = Model.Actions;
module Sneak = Model.Sneak;

module Log = Core.Log;

module SneakRegistry = Oni_UI.SneakRegistry;

let start = () => {
  let discoverSneakEffect =
    Isolinear.Effect.createWithDispatch(~name="sneak.discover", _dispatch => {
      let sneaks = SneakRegistry.getSneaks();
      _dispatch(Model.Actions.Sneak(Sneak.Discovered(sneaks)));
    });

  let completeSneakEffect = (state: Model.State.t) =>
    Isolinear.Effect.createWithDispatch(~name="sneak.discover", dispatch => {
      let filteredSneaks = Sneak.getFiltered(state.sneak);

      switch (filteredSneaks) {
      | [] => dispatch(Model.Actions.Sneak(Sneak.NoneAvailable))
      | [sneak] =>
        let {callback, _}: Sneak.sneak = sneak;
        callback();
        dispatch(Model.Actions.Sneak(Sneak.Executed(sneak)));
      | _ => ()
      };
    });

  let updater = (state: Model.State.t, action) => {
    let default = (state, Isolinear.Effect.none);
    switch (action) {
    | Actions.Command("sneak.start") => (
        {...state, sneak: Model.Sneak.reset(state.sneak)},
        discoverSneakEffect,
      )
    | Actions.Command("sneak.stop") => (
        {...state, sneak: Model.Sneak.hide(state.sneak)},
        Isolinear.Effect.none,
      )
    | Actions.Sneak(sneakAction) =>
      switch (sneakAction) {
      | Sneak.Executed(_)
      | Sneak.NoneAvailable => (
          {...state, sneak: Model.Sneak.hide(state.sneak)},
          Isolinear.Effect.none,
        )
      | Sneak.KeyboardInput(k) =>
        let newState = {...state, sneak: Model.Sneak.refine(k, state.sneak)};
        (newState, completeSneakEffect(newState));
      | Sneak.Discovered(sneaks) => (
          {...state, sneak: Model.Sneak.add(sneaks, state.sneak)},
          Isolinear.Effect.none,
        )
      }
    | _ => default
    };
  };

  updater;
};
