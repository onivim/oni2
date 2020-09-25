open Oni_Core;
open Oni_Core.Utility;

module Log = (val Oni_Core.Log.withNamespace("Oni2.Feature.Configuration"));

module GlobalConfiguration = GlobalConfiguration;

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
        [GlobalConfiguration.contributions, ...contributions]
        |> List.map(Config.Schema.fromList),
      ),
    user: getUserSettings() |> Result.value(~default=Config.Settings.empty),
    merged: Config.Settings.empty,
  });

let toExtensionConfiguration = (config, extensions, setup: Setup.t) => {
  open Exthost.Extension;
  open Scanner.ScanResult;

  let defaults =
    extensions
    |> List.map(ext => ext.manifest.contributes.configuration)
    |> List.map(Contributions.Configuration.toSettings)
    |> Config.Settings.unionMany
    |> Config.Settings.union(Config.Schema.defaults(config.schema))
    |> Exthost.Configuration.Model.fromSettings;

  let user =
    Config.Settings.fromList([
      ("reason_language_server.location", Json.Encode.string(setup.rlsPath)),
      ("telemetry.enableTelemetry", Json.Encode.bool(false)),
      ("terminal.integrated.env.windows", Json.Encode.null),
      ("terminal.integrated.env.linux", Json.Encode.null),
      ("terminal.integrated.env.osx", Json.Encode.null),
    ])
    |> Config.Settings.union(config.user)
    |> Exthost.Configuration.Model.fromSettings;

  Exthost.Configuration.create(~defaults, ~user, ());
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

let vimToCoreSetting =
  fun
  | Vim.Setting.String(str) => VimSetting.String(str)
  | Vim.Setting.Int(i) => VimSetting.Int(i);

let resolver = (model, vimModel, ~vimSetting, key) => {
  // Try to get the vim setting, first...
  let vimResolver = Feature_Vim.Configuration.resolver(vimModel);
  vimSetting
  |> OptionEx.flatMap(vimResolver)
  |> Option.map(setting => Config.Vim(vimToCoreSetting(setting)))
  // If the vim setting isn't set, fall back to our JSON config.
  |> OptionEx.or_lazy(() => {
       Config.Settings.get(key, model.merged)
       |> Option.map(json => Config.Json(json))
     })
  |> Option.value(~default=Config.NotSet);
};
