open Oni_Core;

// MODEL

type model;

let default: model;

let configurationChanged: (Config.resolver, model) => model;

let proxy: model => Service_Net.Proxy.t;

// CONTRIBUTIONS

module Contributions: {let configuration: list(Config.Schema.spec);};
