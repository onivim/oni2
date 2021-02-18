open Oni_Core;
open Exthost;

type model;

// DEPRECATED strategy for working with configuration
module LegacyConfiguration = LegacyConfiguration;
module LegacyConfigurationValues = LegacyConfigurationValues;
module LegacyConfigurationParser = LegacyConfigurationParser;

module Legacy: {
  let configuration: model => LegacyConfiguration.t;
  let getValue:
    (~fileType: string=?, LegacyConfigurationValues.t => 'a, model) => 'a;

  let set: (LegacyConfiguration.t, model) => model;
};

// LOADER

module ConfigurationLoader: {
  type t;

  let none: t;

  let file: Fp.t(Fp.absolute) => t;
};

let initial:
  (~loader: ConfigurationLoader.t, list(list(Config.Schema.spec))) => model;

let toExtensionConfiguration:
  (model, list(Extension.Scanner.ScanResult.t), Setup.t) =>
  Exthost.Configuration.t;

[@deriving show]
type msg =
  | UserSettingsChanged;

type outmsg =
  | ConfigurationChanged({changed: Config.Settings.t})
  | Nothing;

let update: (model, msg) => (model, outmsg);

let notifyFileSaved: (Fp.t(Fp.absolute), model) => model;

let resolver:
  (
    ~fileType: string,
    model,
    Feature_Vim.model,
    ~vimSetting: option(string),
    Config.key
  ) =>
  Config.rawValue;

let sub: model => Isolinear.Sub.t(msg);

module GlobalConfiguration = GlobalConfiguration;
