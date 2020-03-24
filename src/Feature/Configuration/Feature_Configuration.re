open Oni_Core;

module Log = (val Oni_Core.Log.withNamespace("Oni2.Feature.Configuration"));

module UserSettingsProvider = {
  let defaultConfigurationFileName = "configuration.json";

  let getSettings = () =>
    Filesystem.getOrCreateConfigFile(defaultConfigurationFileName)
    |> Result.map(Config.Settings.fromFile);
};

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

let initial = (~getUserSettings, contributions) =>
  merge({
    schema:
      Config.Schema.unionMany(
        contributions |> List.map(Config.Schema.fromList),
      ),
    user: getUserSettings() |> Result.value(~default=Config.Settings.empty),
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
  | UserSettingsChanged;

type outmsg =
  | ConfigurationChanged({changed: Config.Settings.t})
  | Nothing;

let update = (~getUserSettings, model, msg) =>
  switch (msg) {
  | UserSettingsChanged =>
    let previous = model;
    let updated =
      switch (getUserSettings()) {
      | Ok(user) when user == Config.Settings.empty => model // TODO: Not sure why this is needed
      | Ok(user) => merge({...model, user})
      | Error(_) => model
      };

    let changed = Config.Settings.changed(previous.merged, updated.merged);

    (updated, ConfigurationChanged({changed: changed}));
  };

let resolver = (model, key) => Config.Settings.get(key, model.merged);
