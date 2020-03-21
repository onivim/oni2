open Oni_Core;

module Log = (val Oni_Core.Log.withNamespace("Oni2.Feature.Configuration"));

type model = {
  schema: Config.Schema.t,
  user: Config.Settings.t,
  merged: Config.Settings.t,
};

let merge = model => {
  ...model,
  merged:
    Config.Settings.union(Config.Schema.defaults(model.schema), model.user),
};

let initial = contributions =>
  merge({
    schema:
      Config.Schema.unionMany(
        contributions |> List.map(Config.Schema.fromList),
      ),
    user: Config.Settings.empty,
    merged: Config.Settings.empty,
  });

[@deriving show({with_path: false})]
type msg =
  | ConfigurationFileChanged;

let defaultConfigurationFileName = "configuration.json";
let getConfigurationFile = configurationFilePath => {
  switch (configurationFilePath) {
  | None => Filesystem.getOrCreateConfigFile(defaultConfigurationFileName)
  | Some(path) =>
    switch (Sys.file_exists(path)) {
    | exception ex =>
      Log.error("Error loading configuration file at: " ++ path);
      Log.error("  " ++ Printexc.to_string(ex));
      Filesystem.getOrCreateConfigFile(defaultConfigurationFileName);

    | false =>
      Log.error("Error loading configuration file at: " ++ path);
      Filesystem.getOrCreateConfigFile(defaultConfigurationFileName);

    | true => Ok(path)
    }
  };
};

let loadConfiguration = configFile =>
  getConfigurationFile(configFile) |> Result.map(Config.Settings.fromFile);

let update = (~configFile, model, msg) =>
  switch (msg) {
  | ConfigurationFileChanged =>
    switch (loadConfiguration(configFile)) {
    | Ok(user) when user == Config.Settings.empty => model
    | Ok(user) => merge({...model, user})
    | Error(_) => model
    }
  };

let resolver = (model, key) => Config.Settings.get(key, model.merged);
