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
     let sneaks = SneakRegistry.getSneaks()
     List.iter((bbox) => {
      prerr_endline ("NODE BBOX2: " ++ Revery.Math.BoundingBox2d.toString(bbox));
     _dispatch(Model.Actions.Sneak(Sneak.Discover(bbox)));
     }, sneaks);
  } 
  );

  let updater = (state: Model.State.t, action) => {
    let default = (state, Isolinear.Effect.none);
    switch (action) {
    | Model.Actions.Sneak(sneakAction) => 
      switch (sneakAction) {
        | Sneak.Initiated => 
          
          prerr_endline ("start")
          let _ = Oni_UI.SneakRegistry.getSneaks();
          ({
            ...state,
            sneak: Model.Sneak.reset(state.sneak),
          }, discoverSneakEffect);
       | Sneak.Stopped => {
          prerr_endline ("STOPPED!");
          ({
            ...state,
            sneak: Model.Sneak.hide(state.sneak),
          }, Isolinear.Effect.none);
       }
       | Sneak.Discover(bbox) => ({
          ...state,
          sneak: Model.Sneak.add(() => (), bbox, state.sneak),
       }, Isolinear.Effect.none)
      };
    | _ => default
    };
  };

  updater;
};
