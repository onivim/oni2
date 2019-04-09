open Revery_UI;
open Oni_Core;

type layout =
  | VerticalLeft
  | VerticalRight
  | HorizontalTop
  | HorizontalBottom;

module WindowSplitId =
  Revery.UniqueId.Make({});

type split('a) = {
  id: int,
  component: 'a => React.syntheticElement,
  layout,
  /** These values are proportions of the full screen */
  width: option(int),
  height: option(int),
};

type splits('a) = IntMap.t(split('a));

type t('a) = {splits: splits('a)};

let create = (): t('a) => {splits: IntMap.empty};

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
