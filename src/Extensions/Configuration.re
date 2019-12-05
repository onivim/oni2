/*
 * Configuration.re
 *
 * Types related to creating configuration
 *
 */

let emptyJsonObject = `Assoc([]);
let emptyJsonArray = `List([]);

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

  let empty = {contents: emptyJsonObject, keys: []};

  let create = (~keys, contents) => {keys, contents};

  let to_yojson = model => {
    let {contents, keys} = model;

    let keysJson = keys |> List.map(str => `String(str));

    `Assoc([
      ("contents", contents),
      ("keys", `List(keysJson)),
      ("overrides", emptyJsonArray),
    ]);
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
  // configurationScopes: {}
};

let to_yojson = config => {
  let {defaults, user, workspace} = config;
  `Assoc([
    ("defaults", defaults |> Model.to_yojson),
    ("user", user |> Model.to_yojson),
    ("workspace", workspace |> Model.to_yojson),
    ("folders", emptyJsonObject),
    ("isComplete", `Bool(true)),
    ("configurationScopes", emptyJsonObject),
  ]);
};

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
