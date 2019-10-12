/*
 * ConfigurationStoreConnector.re
 *
 * This implements an updater (reducer + side effects) for managing configuration
 */

open Oni_Core;
open Oni_Model;

let start =
    (~configurationFilePath: option(string), ~cliOptions: option(Cli.t)) => {
  let defaultConfigurationFileName = "configuration.json";
  let getConfigurationFile = () => {
    let errorLoading = path => {
      Log.error("Error loading configuration file at: " ++ path);
      Filesystem.getOrCreateConfigFile(defaultConfigurationFileName);
    };

    switch (configurationFilePath) {
    | None => Filesystem.getOrCreateConfigFile(defaultConfigurationFileName)
    | Some(v) =>
      switch (Sys.file_exists(v)) {
      | exception _ => errorLoading(v)
      | false => errorLoading(v)
      | true => Ok(v)
      }
    };
  };

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

  let transformConfigurationEffect = (fileName, transformer) =>
    Isolinear.Effect.createWithDispatch(
      ~name="configuration.transform", dispatch => {
      let configPath = getConfigurationFile();
      switch (configPath) {
      | Error(msg) => Log.error("Unable to load configuration: " ++ msg)
      | Ok(configPath) =>
        Log.perf("Apply configuration transform", () => {
          let parsedJson = Yojson.Safe.from_file(configPath);
          let newJson = transformer(parsedJson);
          let oc = open_out(configPath);
          Yojson.Safe.pretty_to_channel(oc, newJson);
          close_out(oc);
        })
      };
    });

  let reloadConfigurationEffect =
    Isolinear.Effect.createWithDispatch(~name="configuration.reload", dispatch => {
      let configPath = getConfigurationFile();
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
      let configPath = getConfigurationFile();
      switch (configPath) {
      | Ok(configPathAsString) =>
        Log.info(
          "ConfigurationStoreConnector - Loading configuration: "
          ++ configPathAsString,
        );
        switch (ConfigurationParser.ofFile(configPathAsString), cliOptions) {
        | (Ok(configuration), Some(cliOptions)) =>
          dispatch(Actions.ConfigurationSet(configuration));

          let zenModeSingleFile =
            Configuration.getValue(c => c.zenModeSingleFile, configuration);

          if (zenModeSingleFile && List.length(cliOptions.filesToOpen) == 1) {
            dispatch(Actions.EnableZenMode);
          };
        | (Ok(configuration), None) =>
          dispatch(Actions.ConfigurationSet(configuration))
        | (Error(err), _) =>
          Log.error("Error loading configuration file: " ++ err)
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
    | Actions.ConfigurationTransform(file, transformer) => (
        state,
        transformConfigurationEffect(file, transformer),
      )
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
