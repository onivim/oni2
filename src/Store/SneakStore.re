/*
 * SneakStoreConnector.re
 */

module Core = Oni_Core;
module Ext = Oni_Extensions;
module Model = Oni_Model;
module Sneak = Model.Sneak;

module Log = Core.Log;

let start = () => {
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
            sneak: Model.Sneak.setActive(true, state.sneak),
          }, Isolinear.Effect.none);
       | Sneak.Stopped => {
          prerr_endline ("STOPPED!");
          ({
            ...state,
            sneak: Model.Sneak.setActive(false, state.sneak),
          }, Isolinear.Effect.none);
       }
      };
    | _ => default
    };
  };

  updater;
};
