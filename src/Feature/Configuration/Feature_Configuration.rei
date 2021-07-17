open Oni_Core;
open Exthost;

type model;

// LOADER

module ConfigurationLoader: {
  type t;

  let none: t;

  let file: FpExp.t(FpExp.absolute) => t;

  let loadImmediate: t => result(Config.Settings.t, string);
};

let initial:
  (~loader: ConfigurationLoader.t, list(list(Config.Schema.spec))) => model;

let registerExtensionConfigurations:
  (~configurations: list(Extension.Contributions.Configuration.t), model) =>
  model;

let toExtensionConfiguration:
  (
    ~additionalExtensions: list(Exthost.Extension.Scanner.ScanResult.t),
    model
  ) =>
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

let sub:
  (~client: Exthost.Client.t, ~isExthostInitialized: bool, model) =>
  Isolinear.Sub.t(msg);

module GlobalConfiguration = GlobalConfiguration;

// CONTRIBUTIONS

module Contributions: {
  let commands: list(Oni_Core.Command.t(msg));
  let keybindings: list(Feature_Input.Schema.keybinding);
};

module Testing: {let transform: ConfigurationTransformer.t => msg;};
