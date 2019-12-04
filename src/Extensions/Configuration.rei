/*
 * Configuration.rei
 *
 * Types related to modelling extension configuration
 *
 */

// Type relating to 'ConfigurationModel' in VSCode
// This is an 'instance' of configuration - modelling user, workspace, or default configuration.
// The full configuration is set up by combining the various configuration 'instances'.
module Model: {
  type t;

  let empty: t;
  let create: (~keys: list(string), Yojson.Safe.t) => t;
  let to_yojson: t => Yojson.Safe.t;
};

type t;

let to_yojson: t => Yojson.Safe.t;
let empty: t;
let create:
  (~defaults: Model.t=?, ~user: Model.t=?, ~workspace: Model.t=?, unit) => t;
