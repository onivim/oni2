open Revery.Math;

type callback = unit => unit;
type bounds = unit => option(Rectangle.t);

type sneakInfo = {
  callback,
  boundingBox: BoundingBox2d.t,
};

type sneak = {
  callback,
  boundingBox: BoundingBox2d.t,
  id: string,
};

[@deriving show({with_path: false})]
type action =
  | Initiated
  | Discover([@opaque] list(sneakInfo))
  | KeyboardInput(string)
  | Stopped;

type t;

let initial: t;

let getTextHighlight: (string, t) => (string, string);

let reset: t => t;
let hide: t => t;

let isActive: t => bool;

let refine: (string, t) => t;

let add: (list(sneakInfo), t) => t;

let getFiltered: t => list(sneak);
