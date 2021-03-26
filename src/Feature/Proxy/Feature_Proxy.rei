open Oni_Core;

// MODEL

type model = option(Service_Net.Proxy.t);

let initial: model;

let configurationChanged: (Config.resolver, model) => model;

let proxy: model => option(Service_Net.Proxy.t);

// CONTRIBUTIONS

module Contributions: {let configuration: list(Config.Schema.spec);};
