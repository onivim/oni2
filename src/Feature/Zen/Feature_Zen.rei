open Oni_Core;

// MODEL

[@deriving show]
type msg;

type model;

let initial: (~isSingleFile: bool) => model;

let isZen: model => bool;
let shouldShowTabsInZenMode: model => bool;

let exitZenMode: model => model;

let update: (msg, model) => model;

let configurationChanged: (Config.resolver, model) => model;

// CONTRIBUTIONS

module Contributions: {
  let commands: list(Command.t(msg));
  let configuration: list(Config.Schema.spec);
  let contextKeys: model => WhenExpr.ContextKeys.t;
  //let keybindings: list(Feature_Input.Schema.keybinding);
};

// TESTING

module Testing: {let enableZenMode: msg;};
