[@deriving show({with_path: false})]
type action =
  | Initiated;

type sneak = {
  callback: unit => unit,
  boundingBox: Revery.Math.BoundingBox2d.t,
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

let setActive = (active: bool, sneaks: t) => {
  if (active) {
    ...sneaks,
    active
  } else {
    initial
  }
}

let isActive = sneaks => sneaks.active;

let refine = (characterToAdd: string, prefix: string, sneaks: t) => {
  // TODO
};

let add = (callback, boundingBox, sneaks: t) => {
  // TODO
};

let getFiltered = (sneaks: t) => sneaks.filteredSneaks;
