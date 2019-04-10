open Revery_UI;
open Oni_Core;

type layout =
  | VerticalLeft
  | VerticalRight
  | HorizontalTop
  | HorizontalBottom;

module WindowSplitId =
  Revery.UniqueId.Make({});

type componentCreator = unit => React.syntheticElement;

type split = {
  id: int,
  component: componentCreator,
  layout,
  /* if omitted the split will grow to occupy whatever space is available */
  width: option(int),
  height: option(int),
};

type direction =
  | Horizontal
  | Vertical;

type splitTree =
  | Parent(direction, list(splitTree))
  | Leaf(split);

type splits = IntMap.t(split);

type t = {splits};

let create = (): t => {splits: IntMap.empty};

let getId = (id: option(int)) =>
  switch (id) {
  | Some(i) => i
  | None => WindowSplitId.getUniqueId()
  };

let createSplit = (~id=?, ~width=?, ~height=?, ~component, ~layout, ()) => {
  id: getId(id),
  component,
  width,
  height,
  layout,
};

let empty = IntMap.empty;
let add = IntMap.add;
let remove = IntMap.remove;

let toList = map =>
  IntMap.fold((_key, split, accum) => [split, ...accum], map, [])
  |> List.sort((s1, s2) => compare(s1.id, s2.id));
