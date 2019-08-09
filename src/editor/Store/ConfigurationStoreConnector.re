/*
 * ConfigurationStoreConnector.re
 *
 * This implements an updater (reducer + side effects) for managing configuration
 */

open Oni_Core;
open Oni_Model;

let start = (~cliOptions: option(Cli.t)) => {
  let configurationFileName = "configuration.json";
  let reloadConfigOnWritePost = (~configPath, dispatch) => {
    let _ =
      Vim.AutoCommands.onDispatch((cmd, buffer) => {
        let bufferFileName =
          switch (Vim.Buffer.getFilename(buffer)) {
          | None => ""
          | Some(fileName) => fileName
          };
        if (bufferFileName == configPath && cmd == Vim.Types.BufWritePost) {
          dispatch(Actions.ConfigurationReload);
        };
      });
    ();
  };

  let reloadConfigurationEffect =
    Isolinear.Effect.createWithDispatch(~name="configuration.reload", dispatch => {
      let configPath =
        Filesystem.getOrCreateConfigFile(configurationFileName);
      switch (configPath) {
      | Ok(configPathAsString) =>
        switch (ConfigurationParser.ofFile(configPathAsString)) {
        | Ok(v) => dispatch(Actions.ConfigurationSet(v))
        | Error(err) => Log.error("Error loading configuration file: " ++ err)
        }
      | Error(err) => Log.error("Error loading configuration file: " ++ err)
      };
    });

  let initConfigurationEffect =
    Isolinear.Effect.createWithDispatch(~name="configuration.init", dispatch => {
      let configPath =
        Filesystem.getOrCreateConfigFile(configurationFileName);
      switch (configPath) {
      | Ok(configPathAsString) =>
        switch (ConfigurationParser.ofFile(configPathAsString), cliOptions) {
        | (Ok(configuration), Some(cliOptions)) =>
          dispatch(Actions.ConfigurationSet(configuration));

          let zenModeSingleFile =
            Configuration.getValue(c => c.zenModeSingleFile, configuration);

          if (zenModeSingleFile && List.length(cliOptions.filesToOpen) == 1) {
            dispatch(Actions.ToggleZenMode);
          };
        | (Error(err), _) =>
          Log.error("Error loading configuration file: " ++ err)
        | _ => ()
        };
        reloadConfigOnWritePost(~configPath=configPathAsString, dispatch);
      | Error(err) => Log.error("Error loading configuration file: " ++ err)
      };
      ();
    });

  let openConfigurationFileEffect = filePath =>
    Isolinear.Effect.createWithDispatch(
      ~name="configuration.openFile", dispatch =>
      switch (Filesystem.getOrCreateConfigFile(filePath)) {
      | Ok(path) => dispatch(Actions.OpenFileByPath(path, None))
      | Error(e) => Log.error(e)
      }
    );

  let updater = (state: State.t, action: Actions.t) => {
    switch (action) {
    | Actions.Init => (state, initConfigurationEffect)
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
