open Oni_Core;

[@deriving show]
type msg;

type model;

let initial: (~getZoom: unit => float, ~setZoom: float => unit) => model;

let zoom: model => float;

type outmsg =
  | Effect(Isolinear.Effect.t(msg))
  | UpdateConfiguration(ConfigurationTransformer.t)
  | Nothing;

let update: (msg, model) => (model, outmsg);

let configurationChanged:
  (~config: Config.resolver, model) => (model, Isolinear.Effect.t(msg));

module Contributions: {
  let keybindings: list(Feature_Input.Schema.keybinding);
  let commands: list(Command.t(msg));
  let configuration: list(Config.Schema.spec);
};
