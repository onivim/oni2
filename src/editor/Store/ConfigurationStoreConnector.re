/*
 * ConfigurationStoreConnector.re
 *
 * This implements an updater (reducer + side effects) for managing configuration
 */

open Oni_Core;
open Oni_Model;

let start = () => {
  let reloadConfigurationEffect =
    Isolinear.Effect.createWithDispatch(~name="configuration.reload", dispatch => {
      let configPath = Filesystem.getOrCreateConfigFile("configuration.json");
      switch (configPath) {
      | Ok(v) =>
        switch (ConfigurationParser.ofFile(v)) {
        | Ok(v) =>
          dispatch(Actions.ConfigurationSet(v));
        | Error(err) =>
          prerr_endline("Error loading configuration file: " ++ err)
        };
      | Error(err) =>
        prerr_endline("Error loading configuration file: " ++ err)
      };
    });

  let openConfigurationFileEffect = filePath =>
    Isolinear.Effect.createWithDispatch(
      ~name="configuration.openFile", dispatch =>
      switch (Filesystem.getOrCreateConfigFile(filePath)) {
      | Ok(path) => dispatch(Actions.OpenFileByPath(path))
      | Error(e) => prerr_endline(e)
      }
    );

  let updater = (state: State.t, action: Actions.t) => {
    switch (action) {
    | Actions.ConfigurationSet(configuration) => (
        {...state, configuration},
        Isolinear.Effect.none,
      )
    | Actions.ConfigurationReload => (state, reloadConfigurationEffect)
    | Actions.OpenConfigFile(path) => (
        state,
        openConfigurationFileEffect(path),
      )
    | _ => (state, Isolinear.Effect.none)
    };
  };

  updater;
};
