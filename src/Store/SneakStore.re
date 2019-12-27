/*
 * SneakStoreConnector.re
 */

module Core = Oni_Core;
module Ext = Oni_Extensions;
module Model = Oni_Model;
module Sneak = Model.Sneak;

module Log = Core.Log;

module SneakRegistry = Oni_UI.SneakRegistry;

let start = () => {

  let discoverSneakEffect = Isolinear.Effect.createWithDispatch(
  ~name="sneak.discover",
  (_dispatch) => {

     let sneakFromBBox: Revery.Math.BoundingBox2d.t => Sneak.sneakInfo = (boundingBox2d: Revery.Math.BoundingBox2d.t) => {
       Sneak.{
          callback: () => (),
          boundingBox: boundingBox2d,
       };
     };

     let sneaks = SneakRegistry.getSneaks()
     |> List.map(sneakFromBBox);
     _dispatch(Model.Actions.Sneak(Sneak.Discover(sneaks)));
  });

  let updater = (state: Model.State.t, action) => {
    let default = (state, Isolinear.Effect.none);
    switch (action) {
    | Model.Actions.Sneak(sneakAction) => 
      switch (sneakAction) {
        | Sneak.Initiated => 
          
          prerr_endline ("start");
          ({
            ...state,
            sneak: Model.Sneak.reset(state.sneak),
          }, discoverSneakEffect);
       | Sneak.KeyboardInput(k) =>
        ({
          ...state,
          sneak: Model.Sneak.refine(k, state.sneak)
        }, Isolinear.Effect.none)
       | Sneak.Stopped => {
          prerr_endline ("STOPPED!");
          ({
            ...state,
            sneak: Model.Sneak.hide(state.sneak),
          }, Isolinear.Effect.none);
       }
       | Sneak.Discover(sneaks) => ({
          ...state,
          sneak: Model.Sneak.add(sneaks, state.sneak),
       }, Isolinear.Effect.none)
      };
    | _ => default
    };
  };

  updater;
};
