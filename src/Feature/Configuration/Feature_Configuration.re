open Oni_Core;
open Oni_Core.Utility;

module Log = (val Oni_Core.Log.withNamespace("Oni2.Feature.Configuration"));

module GlobalConfiguration = GlobalConfiguration;

module ConfigurationLoader = ConfigurationLoader;

type model = {
  schema: Config.Schema.t,
  defaults: Config.Settings.t,
  user: Config.Settings.t,
  merged: Config.Settings.t,
  legacyConfiguration: LegacyConfiguration.t,
  loader: ConfigurationLoader.t,
  transformTasks: Task.model,
  // Keep track of when a synchronization with the extension host is required
  extHostSyncTick: int,
  // ...and the last settings we merged, so we can determine the right set of
  // changed keys.
  extHostLastConfig: option(Config.Settings.t),
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
  merged: Config.Settings.union(model.defaults, model.user),
};

let initialLoad = model => {
  ConfigurationLoader.loadImmediate(model.loader)
  |> Result.map(((user, legacyConfiguration)) => {
       {...model, user, legacyConfiguration}
     })
  |> Result.value(~default=model);
};

let initial = (~loader, contributions) => {
  let schema =
    Config.Schema.unionMany(
      [GlobalConfiguration.contributions, ...contributions]
      |> List.map(Config.Schema.fromList),
    );
  let defaults = Config.Schema.defaults(schema);
  {
    schema,
    defaults,
    user: Config.Settings.empty,
    merged: Config.Settings.empty,
    legacyConfiguration: LegacyConfiguration.default,
    loader,
    transformTasks: Task.initial(),

    extHostSyncTick: 0,
    extHostLastConfig: None,
  }
  |> initialLoad
  |> merge;
};

let registerExtensionConfigurations =
    (
      ~configurations: list(Exthost_Extension.Contributions.Configuration.t),
      model,
    ) => {
  let defaults =
    configurations
    |> List.map(Exthost_Extension.Contributions.Configuration.toSettings)
    |> Config.Settings.unionMany
    |> Config.Settings.union(model.defaults);

  {...model, defaults, extHostSyncTick: model.extHostSyncTick + 1} |> merge;
};

let toExtensionConfiguration = (~additionalExtensions, model) => {
  open Exthost.Extension;
  open Scanner.ScanResult;

  let configurations =
    additionalExtensions
    |> List.map((extension: Exthost.Extension.Scanner.ScanResult.t) => {
         Exthost.Extension.Manifest.(
           extension.manifest.contributes.configuration
         )
       });

  let model = model |> registerExtensionConfigurations(~configurations);

  let defaults = model.defaults |> Exthost.Configuration.Model.fromSettings;

  let user =
    Config.Settings.fromList([
      ("telemetry.enableTelemetry", Json.Encode.bool(false)),
      ("terminal.integrated.env.windows", Json.Encode.null),
      ("terminal.integrated.env.linux", Json.Encode.null),
      ("terminal.integrated.env.osx", Json.Encode.null),
    ])
    |> Config.Settings.union(model.user)
    |> Exthost.Configuration.Model.fromSettings;

  Exthost.Configuration.create(~defaults, ~user, ());
};

[@deriving show]
type command =
  | OpenConfigurationFile
  | Reload;

[@deriving show]
type testing =
  | Transform([@opaque] ConfigurationTransformer.t);

[@deriving show({with_path: false})]
type msg =
  | Command(command)
  | Testing(testing)
  | Exthost(Exthost.Msg.Configuration.msg)
  | ExthostSyncDispatched({mergedConfig: [@opaque] Config.Settings.t})
  | TransformTask(Task.msg)
  | UserSettingsChanged({
      config: [@opaque] Config.Settings.t,
      legacyConfiguration: [@opaque] LegacyConfiguration.t,
    })
  | ConfigurationParseError(string);

module Msg = {
  let exthost = msg => Exthost(msg);
};

module Testing = {
  let transform = transformer => Testing(Transform(transformer));
};

type outmsg =
  | ConfigurationChanged({changed: Config.Settings.t})
  | OpenFile(FpExp.t(FpExp.absolute))
  | Nothing;

let queueTransform = (~transformer, model) => {
  let task = ConfigurationLoader.transformTask(~transformer, model.loader);

  {...model, transformTasks: Task.queueTask(~task, model.transformTasks)};
};

let update = (model, msg) =>
  switch (msg) {
  | ExthostSyncDispatched({mergedConfig}) => (
      {...model, extHostLastConfig: Some(mergedConfig)},
      Nothing,
    )

  | UserSettingsChanged({config: user, legacyConfiguration}) =>
    let previous = model;
    let updated =
      if (user == Config.Settings.empty) {
        model;
      } else {
        merge({...model, user});
      };

    let changed = Config.Settings.changed(previous.merged, updated.merged);

    (
      {
        ...updated,
        legacyConfiguration,
        extHostSyncTick: model.extHostSyncTick + 1,
      },
      ConfigurationChanged({changed: changed}),
    );

  | Command(OpenConfigurationFile) =>
    let outmsg =
      switch (ConfigurationLoader.getFilePath(model.loader)) {
      | None => Nothing
      | Some(fp) => OpenFile(fp)
      };

    (model, outmsg);

  | Command(Reload) => (
      {...model, loader: ConfigurationLoader.reload(model.loader)},
      Nothing,
    )

  | ConfigurationParseError(_) => (model, Nothing)

  | TransformTask(taskMsg) => (
      {
        ...model,
        loader: ConfigurationLoader.reload(model.loader),
        transformTasks: Task.update(taskMsg, model.transformTasks),
      },
      Nothing,
    )

  | Exthost(exthostMsg) =>
    switch (exthostMsg) {
    | RemoveConfigurationOption({key, _}) =>
      let transformer = ConfigurationTransformer.removeField(key);
      (model |> queueTransform(~transformer), Nothing);
    | UpdateConfigurationOption({key, value, _}) =>
      let transformer = ConfigurationTransformer.setField(key, value);
      (model |> queueTransform(~transformer), Nothing);
    }

  | Testing(Transform(transformer)) => (
      model |> queueTransform(~transformer),
      Nothing,
    )
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

let sub =
    (
      ~client,
      ~isExthostInitialized: bool,
      {loader, transformTasks, merged, extHostSyncTick, extHostLastConfig, _} as model,
    ) => {
  let loaderSub =
    ConfigurationLoader.sub(loader)
    |> Isolinear.Sub.map(
         fun
         | Ok((config, legacyConfiguration)) =>
           UserSettingsChanged({config, legacyConfiguration})
         | Error(msg) => ConfigurationParseError(msg),
       );

  let extHostSyncTick =
    !isExthostInitialized
      ? Isolinear.Sub.none
      : SubEx.task(
          ~name="Feature_Configuration.Sub.ExtHostSync",
          ~uniqueId=extHostSyncTick |> string_of_int,
          ~task=() => {
            let changed =
              extHostLastConfig
              |> Option.map(previous => {
                   Config.Settings.changed(previous, merged)
                 })
              |> Option.value(~default=Config.Settings.empty)
              |> Exthost.Configuration.Model.fromSettings;

            Exthost.Request.Configuration.acceptConfigurationChanged(
              ~configuration=
                toExtensionConfiguration(~additionalExtensions=[], model),
              ~changed,
              client,
            );
          },
        )
        |> Isolinear.Sub.map(() =>
             ExthostSyncDispatched({mergedConfig: model.merged})
           );
  let transformTaskSub =
    Task.sub(transformTasks) |> Isolinear.Sub.map(msg => TransformTask(msg));

  Isolinear.Sub.batch([loaderSub, transformTaskSub, extHostSyncTick]);
};

// COMMANDS

module Commands = {
  open Feature_Commands.Schema;

  let openConfigurationFile =
    define(
      ~category="Preferences",
      ~title="Open configuration file",
      "workbench.action.openSettings",
      Command(OpenConfigurationFile),
    );

  let reload =
    define(
      ~category="Preferences",
      ~title="Reload configuration",
      "workbench.action.reloadSettings",
      Command(Reload),
    );
};

// CONTRIBUTIONS

module Contributions = {
  let commands = Commands.[openConfigurationFile, reload];
};
