open Oni_Core;
open Exthost;


module UserSettingsProvider: {
  let getSettings: unit => result(Config.Settings.t, string);
};

type model;

// DEPRECATED strategy for working with configuration
module LegacyConfiguration = LegacyConfiguration;
module LegacyConfigurationValues = LegacyConfigurationValues;
module LegacyConfigurationParser = LegacyConfigurationParser;

module Legacy:  {
  let configuration: model => LegacyConfiguration.t;
  let getValue: (
    ~fileType: string=?,
    LegacyConfigurationValues.t => 'a,
    model,
  ) => 'a;

  let set: (LegacyConfiguration.t, model) => model;
};

let initial:
  (
    ~getUserSettings: unit => result(Config.Settings.t, string),
    list(list(Config.Schema.spec))
  ) =>
  model;

let toExtensionConfiguration:
  (model, list(Extension.Scanner.ScanResult.t), Setup.t) =>
  Exthost.Configuration.t;

[@deriving show]
type msg =
  | UserSettingsChanged;

type outmsg =
  | ConfigurationChanged({changed: Config.Settings.t})
  | Nothing;

let update:
  (~getUserSettings: unit => result(Config.Settings.t, string), model, msg) =>
  (model, outmsg);

let resolver:
  (
    ~fileType: string,
    model,
    Feature_Vim.model,
    ~vimSetting: option(string),
    Config.key
  ) =>
  Config.rawValue;

module GlobalConfiguration = GlobalConfiguration;
