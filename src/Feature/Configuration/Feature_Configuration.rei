open Oni_Core;
open Exthost.Types;

module UserSettingsProvider: {
  let getSettings: unit => result(Config.Settings.t, string);
};

type model = {
  schema: Config.Schema.t,
  user: Config.Settings.t,
  merged: Config.Settings.t,
};

let initial:
  (
    ~getUserSettings: unit => result(Config.Settings.t, string),
    list(list(Config.Schema.spec))
  ) =>
  model;

let toExtensionConfiguration:
  (model, list(Exthost.Extension.Scanner.ScanResult.t), Setup.t) =>
  Oni_Extensions.Configuration.t;

[@deriving show]
type msg =
  | UserSettingsChanged;

type outmsg =
  | ConfigurationChanged({changed: Config.Settings.t})
  | Nothing;

let update:
  (~getUserSettings: unit => result(Config.Settings.t, string), model, msg) =>
  (model, outmsg);

let resolver: (model, Config.key) => option(Json.t);
