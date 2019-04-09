open Revery_UI;

module Core = Oni_Core;

type layout =
  | VerticalLeft
  | VerticalRight
  | HorizontalTop
  | HorizontalBottom
  | Full;

type split = {
  id: int,
  /**
     TODO: State cannot be passed in here as it leads to a circular
     dependency instead we can only explicitly pass in what we need
   */
  component: unit => React.syntheticElement,
  layout,
  /** These values are proportions of the full screen */
  width: int,
  height: int,
};

type splits = Core.IntMap.t(split);

type t = {splits};

let empty = Core.IntMap.empty;
let add = Core.IntMap.add;
let remove = Core.IntMap.remove;
let toList = map =>
  Core.IntMap.fold((_, split, allSplits) => [split, ...allSplits], map, []);

let create = () => {splits: Core.IntMap.empty};
