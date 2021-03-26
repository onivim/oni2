open Oni_Core;

type model = unit;

let initial = ();

let configurationChanged = (_config, _model) => _model;

let proxy = _model => None;

module Contributions = {
  let configuration = [];
};
