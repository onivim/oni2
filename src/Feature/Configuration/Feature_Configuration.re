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

let toExtensionConfiguration = (config, extensions, setup: Setup.t) => {
  open Oni_Extensions;

  let defaults =
    extensions
    |> List.map(ext =>
         ext.ExtensionScanner.manifest.contributes.configuration
       )
    |> List.map(ExtensionContributions.Configuration.toSettings)
    |> Config.Settings.unionMany
    |> Config.Settings.union(Config.Schema.defaults(config.schema))
    |> Configuration.Model.fromSettings;

  let user =
    Config.Settings.fromList([
      ("reason_language_server.location", Json.Encode.string(setup.rlsPath)),
      ("terminal.integrated.env.windows", Json.Encode.null),
      ("terminal.integrated.env.linux", Json.Encode.null),
      ("terminal.integrated.env.osx", Json.Encode.null),
    ])
    |> Config.Settings.union(config.user)
    |> Configuration.Model.fromSettings;

  Configuration.create(~defaults, ~user, ());
};

[@deriving show({with_path: false})]
type msg =
  | ConfigurationFileChanged;

type outmsg =
  | ConfigurationChanged({changed: Config.Settings.t})
  | Nothing;

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
    let previous = model;
    let updated =
      switch (loadConfiguration(configFile)) {
      | Ok(user) when user == Config.Settings.empty => model
      | Ok(user) => merge({...model, user})
      | Error(_) => model
      };

    let changed = Config.Settings.changed(previous.merged, updated.merged);

    (updated, ConfigurationChanged({changed: changed}));
  };

let resolver = (model, key) => Config.Settings.get(key, model.merged);
