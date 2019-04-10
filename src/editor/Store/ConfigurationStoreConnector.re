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
    prerr_endline ("LOADED CONFIGURATION: " ++ Configuration.show(configuration));
    dispatch(Actions.ConfigurationSet(configuration));
  });

  let openConfigurationFileEffect = (filePath) => Isolinear.Effect.createWithDispatch(~name="configuration.openFile", (dispatch) => {
        switch (Filesystem.createOniConfigFile(filePath)) {        
        | Ok(path) => dispatch(Actions.OpenFileByPath(path))
        | Error(e) => prerr_endline(e)
        };
        });

  let updater = (state: State.t, action: Actions.t) => {
    switch (action) {
    | Actions.ConfigurationSet(configuration) => ({
        ...state,
        configuration,
        }, Isolinear.Effect.none) 
    | Actions.ConfigurationReload => (state, reloadConfigurationEffect) 
    | Actions.OpenConfigFile(path) => (state, openConfigurationFileEffect(path))
    | _ => (state, Isolinear.Effect.none)
    };
  };

  updater
};
