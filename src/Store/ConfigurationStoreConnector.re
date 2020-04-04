/*
 * ConfigurationStoreConnector.re
 *
 * This implements an updater (reducer + side effects) for managing configuration
 */

open Oni_Core;
open Oni_Model;

module Log = (val Log.withNamespace("Oni2.Store.Configuration"));

let start =
    (
      ~configurationFilePath: option(string),
      ~cliOptions: Cli.t,
      ~getZoom,
      ~setZoom,
      ~setVsync,
    ) => {
  let defaultConfigurationFileName = "configuration.json";

  let getConfigurationFile = fileName => {
    Filesystem.getOrCreateConfigFile(
      ~overridePath=?configurationFilePath,
      fileName,
    );
  };

  let reloadConfigOnWritePost = (~configPath, dispatch) => {
    let _: unit => unit =
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

  let transformConfigurationEffect = (fileName, buffers, transformer) => {
    let configPath = getConfigurationFile(fileName);
    switch (configPath) {
    | Error(msg) =>
      Log.error("Unable to load configuration: " ++ msg);
      Isolinear.Effect.none;
    | Ok(configPath) =>
      if (!Buffers.isModifiedByPath(buffers, configPath)) {
        Oni_Core.Log.perf("Apply configuration transform", () => {
          let parsedJson = Yojson.Safe.from_file(configPath);
          let newJson = transformer(parsedJson);
          let oc = open_out(configPath);
          Yojson.Safe.pretty_to_channel(oc, newJson);
          close_out(oc);
        });

        Isolinear.Effect.none;
      } else {
        {
          Feature_Notification.Effects.create(
            ~kind=Error,
            "Unable to save theme selection to configuration; configuration file is modified.",
          );
        }
        |> Isolinear.Effect.map(msg => Actions.Notification(msg));
      }
    };
  };

  let reloadConfigurationEffect =
    Isolinear.Effect.createWithDispatch(~name="configuration.reload", dispatch => {
      dispatch(Actions.Configuration(UserSettingsChanged));
      defaultConfigurationFileName
      |> getConfigurationFile
      |> (
        result =>
          Stdlib.Result.bind(result, ConfigurationParser.ofFile)
          |> (
            fun
            | Ok(config) => dispatch(Actions.ConfigurationSet(config))
            | Error(err) =>
              Log.error("Error loading configuration file: " ++ err)
          )
      );
    });

  let initConfigurationEffect =
    Isolinear.Effect.createWithDispatch(~name="configuration.init", dispatch =>
      if (cliOptions.shouldLoadConfiguration) {
        dispatch(Actions.Configuration(UserSettingsChanged));
        switch (getConfigurationFile(defaultConfigurationFileName)) {
        | Ok(path) =>
          Log.info("Loading configuration: " ++ path);

          switch (ConfigurationParser.ofFile(path)) {
          | Ok(configuration) =>
            dispatch(Actions.ConfigurationSet(configuration));

            let zenModeSingleFile =
              Configuration.getValue(c => c.zenModeSingleFile, configuration);

            if (zenModeSingleFile && List.length(cliOptions.filesToOpen) == 1) {
              dispatch(Actions.EnableZenMode);
            };
          | Error(err) =>
            Log.error("Error loading configuration file: " ++ err)
          };
          reloadConfigOnWritePost(~configPath=path, dispatch);
        | Error(err) => Log.error("Error loading configuration file: " ++ err)
        };
        ();
      } else {
        Log.info("Not loading configuration initially; disabled via CLI.");
      }
    );

  let openConfigurationFileEffect = filePath =>
    Isolinear.Effect.createWithDispatch(
      ~name="configuration.openFile", dispatch =>
      switch (Filesystem.getOrCreateConfigFile(filePath)) {
      | Ok(path) => dispatch(Actions.OpenFileByPath(path, None, None))
      | Error(e) => Log.error(e)
      }
    );

  // Synchronize miscellaneous configuration settings
  let zoom = ref(getZoom());
  let vsync = ref(Revery.Vsync.Immediate);
  let synchronizeConfigurationEffect = configuration =>
    Isolinear.Effect.create(~name="configuration.synchronize", () => {
      let zoomValue =
        max(1.0, Configuration.getValue(c => c.uiZoom, configuration));
      if (zoomValue != zoom^) {
        Log.infof(m => m("Setting zoom: %f", zoomValue));
        setZoom(zoomValue);
        zoom := zoomValue;
      };

      let vsyncValue = Configuration.getValue(c => c.vsync, configuration);
      if (vsyncValue != vsync^) {
        Log.info("Setting vsync: " ++ Revery.Vsync.toString(vsyncValue));
        setVsync(vsyncValue);
        vsync := vsyncValue;
      };
    });

  let updater = (state: State.t, action: Actions.t) => {
    switch (action) {
    | Actions.Init => (state, initConfigurationEffect)
    | Actions.ConfigurationTransform(file, transformer) => (
        state,
        transformConfigurationEffect(file, state.buffers, transformer),
      )
    | Actions.ConfigurationSet(configuration) => (
        {...state, configuration},
        synchronizeConfigurationEffect(configuration),
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
