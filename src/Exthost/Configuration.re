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

// Must be kept in-sync with:
// https://github.com/onivim/vscode-exthost/blob/c7df89c1cf0087ca5decaf8f6d4c0fd0257a8b7a/src/vs/platform/configuration/common/configuration.ts#L30
module Target = {
  [@deriving show]
  type t =
    | User
    | UserLocal
    | UserRemote
    | Workspace
    | WorkspaceFolder
    | Default
    | Memory;

  let toInt =
    fun
    | User => 1
    | UserLocal => 2
    | UserRemote => 3
    | Workspace => 4
    | WorkspaceFolder => 5
    | Default => 6
    | Memory => 7;

  let ofInt =
    fun
    | 1 => Some(User)
    | 2 => Some(UserLocal)
    | 3 => Some(UserRemote)
    | 4 => Some(Workspace)
    | 5 => Some(WorkspaceFolder)
    | 6 => Some(Default)
    | 7 => Some(Memory)
    | _ => None;

  let toString =
    fun
    | User => "USER"
    | UserLocal => "USER_LOCAL"
    | UserRemote => "USER_REMOTE"
    | Workspace => "WORKSPACE"
    | WorkspaceFolder => "WORKSPACE_FOLDER"
    | Default => "DEFAULT"
    | Memory => "MEMORY";

  let encode = target => target |> toInt |> Json.Encode.int;

  let decode =
    Json.Decode.(
      int
      |> map(ofInt)
      |> and_then(
           fun
           | Some(target) => succeed(target)
           | None => fail("Unable to parse target"),
         )
    );
};

module Overrides = {
  [@deriving show]
  type t = {
    overrideIdentifier: option(string),
    resource: option(Oni_Core.Uri.t),
  };

  let decode =
    Json.Decode.(
      obj(({field, _}) =>
        {
          overrideIdentifier: field.optional("overrideIdentifier", string),
          resource:
            field.withDefault(
              "resource",
              None,
              nullable(Oni_Core.Uri.decode),
            ),
        }
      )
    );
};
