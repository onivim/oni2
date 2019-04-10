/*
 * ConfigurationStoreConnector.re
 *
 * This implements an updater (reducer + side effects) for managing configuration
 */

open Oni_Core;
open Oni_Model;

let start = (setup: Setup.t) => {
  let reloadConfigurationEffect = Isolinear.Effect.createWithDispatch(~name="configuration.reload", (dispatch) => {
    let configuration = Configuration.create(~configPath=setup.configPath, ()); 
    dispatch(Actions.ConfigurationSet(configuration));
  });

  let updater = (state: State.t, action: Actions.t) => {
    switch (action) {
    | Actions.ConfigurationSet(configuration) => ({
        ...state,
        configuration,
        }, Isolinear.Effect.none) 
    | Actions.ConfigurationReload => (state, reloadConfigurationEffect) 
    | _ => (state, Isolinear.Effect.none)
    };
  };

  updater
};
