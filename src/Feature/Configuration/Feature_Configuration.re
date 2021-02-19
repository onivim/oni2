open Oni_Core;
open Oni_Core.Utility;

module Log = (val Oni_Core.Log.withNamespace("Oni2.Feature.Configuration"));

module GlobalConfiguration = GlobalConfiguration;

module ConfigurationLoader = ConfigurationLoader;

type model = {
  schema: Config.Schema.t,
  user: Config.Settings.t,
  merged: Config.Settings.t,
  legacyConfiguration: LegacyConfiguration.t,
  loader: ConfigurationLoader.t,
};

// DEPRECATED way of working with configuration
module LegacyConfiguration = LegacyConfiguration;
module LegacyConfigurationValues = LegacyConfigurationValues;
module LegacyConfigurationParser = LegacyConfigurationParser;

module Legacy = {
  let configuration = ({legacyConfiguration, _}) => legacyConfiguration;

  let getValue = (~fileType=?, selector, {legacyConfiguration, _}) => {
    LegacyConfiguration.getValue(~fileType?, selector, legacyConfiguration);
  };

  let set = (legacyConfiguration, model) => {...model, legacyConfiguration};
};

let merge = model => {
  ...model,
  merged:
    Config.Settings.union(Config.Schema.defaults(model.schema), model.user),
};

let initialLoad = model => {
  ConfigurationLoader.loadImmediate(model.loader)
  |> Result.map(((user, legacyConfiguration)) => {
       {...model, user, legacyConfiguration}
     })
  |> Result.value(~default=model);
};

let initial = (~loader, contributions) =>
  {
    schema:
      Config.Schema.unionMany(
        [GlobalConfiguration.contributions, ...contributions]
        |> List.map(Config.Schema.fromList),
      ),
    user: Config.Settings.empty,
    merged: Config.Settings.empty,
    legacyConfiguration: LegacyConfiguration.default,
    loader,
  }
  |> initialLoad
  |> merge;

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
  | UserSettingsChanged({
    config: [@opaque] Config.Settings.t,
    legacyConfiguration: [@opaque] LegacyConfiguration.t,
  })
  | ConfigurationParseError(string);

type outmsg =
  | ConfigurationChanged({changed: Config.Settings.t})
  | Nothing;

let update = (model, msg) =>
  switch (msg) {
  | UserSettingsChanged({config: user, legacyConfiguration}) =>
      //prerr_endline ("!!USER SETTINGS CHANGED");
      let previous = model;
      let updated = if (user == Config.Settings.empty) {
        //prerr_endline ("Empty :(");
        model
      } else {
        //prerr_endline ("MERGING :(");
        merge({...model, user})
      };

    let changed = Config.Settings.changed(previous.merged, updated.merged);

    ({...updated, legacyConfiguration }, ConfigurationChanged({changed: changed}));

  | ConfigurationParseError(_) => 
    // TODO: Bring back diagnostics
    (model, Nothing);
  };

// TODO:
let notifyFileSaved = (path, model) => {
  ...model,
  loader: ConfigurationLoader.notifyFileSaved(path, model.loader),
};

let vimToCoreSetting =
  fun
  | Vim.Setting.String(str) => VimSetting.String(str)
  | Vim.Setting.Int(i) => VimSetting.Int(i);

let resolver = (~fileType: string, model, vimModel, ~vimSetting, key) => {
  // Try to get the vim setting, first...
  let vimResolver = Feature_Vim.Configuration.resolver(vimModel);
  vimSetting
  |> OptionEx.flatMap(vimResolver)
  |> Option.map(setting => Config.Vim(vimToCoreSetting(setting)))
  // If the vim setting isn't set, fall back to our JSON config.
  |> OptionEx.or_lazy(() => {
       // Try to get the value from a per-filetype config first..
       let fileTypeKey = Config.key("[" ++ fileType ++ "]");

       // Fetch the filetype section...
       Config.Settings.get(fileTypeKey, model.merged)
       |> Option.map(Config.Settings.fromJson)
       |> OptionEx.flatMap(fileTypeModel =>
            Config.Settings.get(key, fileTypeModel)
          )
       |> Option.map(json => Config.Json(json))
       // And if this failed...
       |> OptionEx.or_lazy(() => {
            // ...fall back to getting original config
            Config.Settings.get(key, model.merged)
            |> Option.map(json => Config.Json(json))
          });
     })
  |> Option.value(~default=Config.NotSet);
};

let sub = ({loader, _}) => {
  ConfigurationLoader.sub(loader)
  |> Isolinear.Sub.map(fun
  | Ok((config, legacyConfiguration)) => UserSettingsChanged({config: config, legacyConfiguration})
  | Error(msg) => ConfigurationParseError(msg));
};
