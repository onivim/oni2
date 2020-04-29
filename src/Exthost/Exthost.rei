open Oni_Core;

module Extension = Exthost_Extension;
module Protocol = Exthost_Protocol;
module Transport = Exthost_Transport;

module Configuration: {

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
};

module NamedPipe: {
  type t;

  let create: string => t;
  let toString: t => string;
};
