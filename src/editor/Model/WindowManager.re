open Oni_Core;
open Types.EditorSplits;

type t = {splits};

let empty = IntMap.empty;

let add = IntMap.add;

let remove = IntMap.remove;

let toList = map =>
  IntMap.fold((_, split, allSplits) => [split, ...allSplits], map, []);

let create = () => {splits: IntMap.empty};

let reduce = (state: t, action: Actions.t) =>
  switch (action) {
  | AddSplit(split) => {splits: IntMap.add(split.id, split, state.splits)}
  | RemoveSplit(id) => {splits: IntMap.remove(id, state.splits)}
  | _ => state
  };
