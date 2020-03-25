/*
 * Configuration.rei
 *
 * Types related to modelling extension configuration
 *
 */

open Oni_Core;

// Type relating to 'ConfigurationModel' in VSCode
// This is an 'instance' of configuration - modelling user, workspace, or default configuration.
// The full configuration is set up by combining the various configuration 'instances'.
module Model: {
  type t;

  let empty: t;
  let create: (~keys: list(string), Json.t) => t;
  let to_yojson: t => Json.t;
  let encode: Json.encoder(t);

  let fromSettings: Config.Settings.t => t;
  let toString: t => string;
};

type t;

let to_yojson: t => Json.t;
let encode: Json.encoder(t);
let empty: t;
let create:
  (~defaults: Model.t=?, ~user: Model.t=?, ~workspace: Model.t=?, unit) => t;
