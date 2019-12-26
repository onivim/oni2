[@deriving show({with_path: false})]
type action =
  | Initiated;

type sneak = {
  callback: unit => unit,
  boundingBox: Revery.Match.BoundingBox2d.t,
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

let refine = (characterToAdd: string, prefix: sneaks: t) => {
  // TODO
};

let add = (callback, boundingBox, sneaks: t) => {
  // TODO
};

let getFiltered = (sneaks: t) => filteredSneaks;
