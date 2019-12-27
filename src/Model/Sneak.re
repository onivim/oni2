open Revery.Math;

type callback = unit => unit;

[@deriving show({with_path: false})]
type action =
  | Initiated
  | Stopped
  | Discover([@opaque] BoundingBox2d.t);

type sneak = {
  callback: callback,
  boundingBox: BoundingBox2d.t,
  id: string,
}

type t = {
  active: bool,
  allSneaks: list(sneak),
  prefix: string,
  filteredSneaks: list(sneak),
};

let initial: t = {
  active: false,
  allSneaks: [],
  prefix: "",
  filteredSneaks: [],
};

let reset = (_sneak) => {
    ...initial,
    active: true,
};

let hide = (_sneak) => initial;

let isActive = sneaks => sneaks.active;

let _filter = (_prefix: string, _sneak: sneak) => {
  // StringUtil.contains(prefix, sneak.id);
  true
};

let _applyFilter = (sneaks: t) => {
  ...sneaks,
  filteredSneaks: List.filter(_filter(sneaks.prefix), sneaks.filteredSneaks),
}

let refine = (characterToAdd: string, prefix: string, sneaks: t) => {
  // TODO
};

let add = (callback, boundingBox, sneaks: t) => {
  let allSneaks = [{callback, boundingBox, id: "AA"}, ...sneaks.allSneaks];
  let filteredSneaks = allSneaks;
  {
  ...sneaks,
  allSneaks,
  filteredSneaks,
  }
};

let getFiltered = (sneaks: t) => sneaks.filteredSneaks;
