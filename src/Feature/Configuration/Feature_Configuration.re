open Oni_Core;

module Log = (val Oni_Core.Log.withNamespace("Oni2.Feature.Configuration"));

module Globals = GlobalConfiguration;

type model = {settings: Config.t};

let defaults =
  [
    Globals.Editor.defaults,
    Globals.Files.defaults,
    Globals.Vim.defaults,
    Globals.Workbench.defaults,
  ]
  |> List.map(Config.fromList)
  |> Config.unionMany;

let initial = {settings: Config.empty};

[@deriving show({with_path: false})]
type msg =
  | ConfigurationFileChanged;

let getConfigurationFile = Filesystem.getOrCreateConfigFile;

let update = (~configFile, model, msg) =>
  switch (msg) {
  | ConfigurationFileChanged =>
    switch (getConfigurationFile(configFile)) {
    | Ok(configFile) =>
      let settings = Config.fromFile(configFile);
      settings == Config.empty ? model : {settings: settings};
    | Error(_) => model
    }
  };
