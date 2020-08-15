open Oni_Core;

type provider = {
  handle: int,
  selector: Exthost.DocumentSelector.t,
};

type lenses = {lenses: list(Exthost.CodeLens.t)};

type model = {
  providers: list(provider),
  bufferToLenses: IntMap.t(lenses),
};

let initial = {providers: [], bufferToLenses: IntMap.empty};

[@deriving show]
type msg = unit;
// TODO: Hook up subscription
//  | CodelensesReceived({
//      bufferId: int,
//      lenses: list(Exthost.CodeLens.t),
//    });

let register = (~handle: int, ~selector, model) => {
  ...model,
  providers: [{handle, selector}, ...model.providers],
};

let unregister = (~handle: int, model) => {
  ...model,
  providers: model.providers |> List.filter(prov => prov.handle != handle),
};

let update = (_msg, model) => model;

module Sub = {
  let create = (~visibleBuffers, ~client) => {
    ignore(visibleBuffers);
    ignore(client);
    Isolinear.Sub.none;
  };
};

let sub = Sub.create;
