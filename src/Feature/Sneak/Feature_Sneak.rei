open Revery.Math;

type sneakInfo = {
  callback: unit => unit,
  boundingBox: BoundingBox2d.t,
};

type sneak = {
  callback: unit => unit,
  boundingBox: BoundingBox2d.t,
  id: string,
};

[@deriving show({with_path: false})]
type msg =
  | NoneAvailable
  | Executed([@opaque] sneak)
  | Discovered([@opaque] list(sneakInfo))
  | KeyboardInput(string);

type model;

let initial: model;

let getTextHighlight: (string, model) => (string, string);

let reset: model => model;
let hide: model => model;

let isActive: model => bool;

let refine: (string, model) => model;

let add: (list(sneakInfo), model) => model;

let getFiltered: model => list(sneak);

// REGISTRY

module Registry: {
  let getSneaks: unit => list(sneakInfo);

  let register: (ref(option(Revery.UI.node)), unit => unit) => unit;
  let unregister: ref(option(Revery.UI.node)) => unit;
};
