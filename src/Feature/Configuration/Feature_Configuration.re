open Oni_Core;

module Log = (val Oni_Core.Log.withNamespace("Oni2.Feature.Configuration"));

type model = {settings: Config.t};

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
