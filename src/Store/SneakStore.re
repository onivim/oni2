/*
 * SneakStoreConnector.re
 */

module Core = Oni_Core;
module Ext = Oni_Extensions;
module Model = Oni_Model;
module Actions = Model.Actions;
module Sneak = Model.Sneak;

module SneakRegistry = Feature_Sneak.Registry;

let start = () => {
  let discoverSneakEffect =
    Isolinear.Effect.createWithDispatch(~name="sneak.discover", _dispatch => {
      let sneaks = SneakRegistry.getSneaks();
      _dispatch(Model.Actions.Sneak(Feature_Sneak.Discovered(sneaks)));
    });

  let completeSneakEffect = (state: Model.State.t) =>
    Isolinear.Effect.createWithDispatch(~name="sneak.discover", dispatch => {
      let filteredSneaks = Feature_Sneak.getFiltered(state.sneak);

      switch (filteredSneaks) {
      | [] => dispatch(Model.Actions.Sneak(Feature_Sneak.NoneAvailable))
      | [sneak] =>
        let {callback, _}: Feature_Sneak.sneak = sneak;
        callback();
        dispatch(Model.Actions.Sneak(Feature_Sneak.Executed(sneak)));
      | _ => ()
      };
    });

  let updater = (state: Model.State.t, action) => {
    let default = (state, Isolinear.Effect.none);
    switch (action) {
    | Actions.Command("sneak.start") => (
        {...state, sneak: Feature_Sneak.reset(state.sneak)},
        discoverSneakEffect,
      )
    | Actions.Command("sneak.stop") => (
        {...state, sneak: Feature_Sneak.hide(state.sneak)},
        Isolinear.Effect.none,
      )
    | Actions.Sneak(sneakAction) =>
      switch (sneakAction) {
      | Feature_Sneak.Executed(_)
      | Feature_Sneak.NoneAvailable => (
          {...state, sneak: Feature_Sneak.hide(state.sneak)},
          Isolinear.Effect.none,
        )
      | Feature_Sneak.KeyboardInput(k) =>
        let newState = {
          ...state,
          sneak: Feature_Sneak.refine(k, state.sneak),
        };
        (newState, completeSneakEffect(newState));
      | Feature_Sneak.Discovered(sneaks) => (
          {...state, sneak: Feature_Sneak.add(sneaks, state.sneak)},
          Isolinear.Effect.none,
        )
      }
    | _ => default
    };
  };

  updater;
};
