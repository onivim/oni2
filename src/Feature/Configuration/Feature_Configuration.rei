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

  let file: FpExp.t(FpExp.absolute) => t;
};

let initial:
  (~loader: ConfigurationLoader.t, list(list(Config.Schema.spec))) => model;

let toExtensionConfiguration:
  (model, list(Extension.Scanner.ScanResult.t), Setup.t) =>
  Exthost.Configuration.t;

[@deriving show]
type msg;

module Msg: {let exthost: Exthost.Msg.Configuration.msg => msg;};

type outmsg =
  | ConfigurationChanged({changed: Config.Settings.t})
  | OpenFile(FpExp.t(FpExp.absolute))
  | Nothing;

let update: (model, msg) => (model, outmsg);

let notifyFileSaved: (FpExp.t(FpExp.absolute), model) => model;

let queueTransform: (~transformer: ConfigurationTransformer.t, model) => model;

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

// CONTRIBUTIONS

module Contributions: {let commands: list(Oni_Core.Command.t(msg));};

module Testing: {let transform: ConfigurationTransformer.t => msg;};
