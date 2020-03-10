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

let getConfigurationFile = Filesystem.getOrCreateConfigFile;

let update = (~configFile, model, msg) =>
  switch (msg) {
  | ConfigurationFileChanged =>
    switch (getConfigurationFile(configFile)) {
    | Ok(configFile) =>
      let user = Config.Settings.fromFile(configFile);
      user == Config.Settings.empty ? model : merge({...model, user});
    | Error(_) => model
    }
  };
