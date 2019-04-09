open Revery_UI;
open Oni_Core;

type layout =
  | VerticalLeft
  | VerticalRight
  | HorizontalTop
  | HorizontalBottom
  | Full;

type split('a) = {
  id: int,
  component: 'a => React.syntheticElement,
  layout,
  /** These values are proportions of the full screen */
  width: int,
  height: int,
};

type splits('a) = IntMap.t(split('a));

type t('a) = {splits: splits('a)};

let create = (): t('a) => {splits: IntMap.empty};

let empty = IntMap.empty;
let add = IntMap.add;
let remove = IntMap.remove;
let toList = map =>
  IntMap.fold((_, split, accum) => [split, ...accum], map, []);
