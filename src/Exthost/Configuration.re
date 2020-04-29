/*
 * Configuration.re
 *
 * Types related to creating configuration
 *
 */

 open Oni_Core;

// Type relating to 'ConfigurationModel' in VSCode
// This is an 'instance' of configuration - modelling user, workspace, or default configuration.
// The full configuration is set up by combining the various configuration 'instances'.
module Model = {
  type t = {
    contents: Yojson.Safe.t,
    keys: list(string),
    // TODO: Investigate hooking up the 'overrides' setting
    // overrides: ?
  };

  let empty = {contents: Json.Encode.obj([]), keys: []};

  let create = (~keys, contents) => {keys, contents};

  let encode = model =>
    Json.Encode.(
      obj([
        ("contents", model.contents),
        ("keys", model.keys |> list(string)),
        ("overrides", list(value, [])),
      ])
    );

  let fromSettings = settings => {
    keys: settings |> Config.Settings.keys |> List.map(Config.keyAsString),
    contents: settings |> Config.Settings.toJson,
  };


  let to_yojson = Json.Encode.encode_value(encode);

  let toString = (model: t) => {
    Printf.sprintf(
      "Keys: %s \n JSON: %s\n",
      model.keys |> String.concat("\n"),
      model.contents |> Yojson.Safe.to_string,
    );
  };
};

type t = {
  defaults: Model.t,
  user: Model.t,
  workspace: Model.t,
  // TODO: Investigate 'isComplete' option here
  //isComplete: bool,
  // TODO: Investigate 'folders' option here
  // folders: ?
  // TODO: Investigate 'configurationScopes' option here
  // configurationScopes: []
};

let encode = config =>
  Json.Encode.(
    obj([
      ("defaults", config.defaults |> Model.encode),
      ("user", config.user |> Model.encode),
      ("workspace", config.workspace |> Model.encode),
      ("folders", list(bool, [])),
      ("isComplete", bool(true)),
      ("configurationScopes", list(bool, [])),
    ])
  );

let to_yojson = Json.Encode.encode_value(encode);

let empty = {
  defaults: Model.empty,
  user: Model.empty,
  workspace: Model.empty,
};

let create =
    (~defaults=Model.empty, ~user=Model.empty, ~workspace=Model.empty, ()) => {
  // For now... we'll only consider a single configuration model. But we'll need to update this to handle
  // workspace / user configuration for better fidelity with extensions, later!
  defaults,
  user,
  workspace,
};
