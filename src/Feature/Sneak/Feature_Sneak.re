open Revery.Math;

module InputModel = Oni_Components.InputModel;
module StringEx = Oni_Core.Utility.StringEx;
module Selection = Oni_Components.Selection;

type callback = unit => unit;
type bounds = unit => option(BoundingBox2d.t);

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
type msg =
  | NoneAvailable
  | Executed([@opaque] sneak)
  | Discovered([@opaque] list(sneakInfo))
  | KeyboardInput(string);

type model = {
  active: bool,
  allSneaks: list(sneak),
  prefix: string,
  filteredSneaks: list(sneak),
};

let initial: model = {
  active: false,
  allSneaks: [],
  prefix: "",
  filteredSneaks: [],
};

let getTextHighlight = (text: string, model) => {
  let prefixLength = model.prefix |> String.length;
  let idLength = text |> String.length;

  let remainder = String.sub(text, prefixLength, idLength - prefixLength);

  (model.prefix, remainder);
};

let reset = _sneak => {...initial, active: true};

let hide = _sneak => initial;

let isActive = sneaks => sneaks.active;

module Internal = {
  let filter = (prefix: string, sneak: sneak) => {
    StringEx.startsWith(~prefix, sneak.id);
  };

  let applyFilter = (sneaks: model) =>
    if (sneaks.prefix == "") {
      {...sneaks, filteredSneaks: sneaks.allSneaks};
    } else {
      {
        ...sneaks,
        filteredSneaks: List.filter(filter(sneaks.prefix), sneaks.allSneaks),
      };
    };

  // Ported from: https://github.com/onivim/oni/blob/74a4dc7f2240a1f5f7a799b2f3f9d01d69b01bac/browser/src/Services/Sneak/SneakStore.ts#L95
  // But could be improved:
  // - Preference for home row
  let getLabelFromIndex = (i: int) => {
    let aChar = Char.code('A');
    let firstDigit = i / 26;
    let secondDigit = i - firstDigit * 26;

    let firstChar = Char.chr(firstDigit + aChar);
    let secondChar = Char.chr(secondDigit + aChar);
    String.make(1, firstChar) ++ String.make(1, secondChar);
  };
};

let refine = (characterToAdd: string, sneaks: model) => {
  let characterToAdd = String.uppercase_ascii(characterToAdd);
  let selection =
    String.length(sneaks.prefix) |> Selection.collapsed(~text=sneaks.prefix);

  let (prefix, _) =
    InputModel.handleInput(~text=sneaks.prefix, ~selection, characterToAdd);

  {...sneaks, prefix} |> Internal.applyFilter;
};

let add = (sneaksToAdd: list(sneakInfo), sneaks: model) => {
  let toSneakInfo = (index: int, sneak: sneakInfo) => {
    {
      boundingBox: sneak.boundingBox,
      callback: sneak.callback,
      id: Internal.getLabelFromIndex(index),
    };
  };

  let sort = (sneakA: sneakInfo, sneakB: sneakInfo) => {
    let bboxA = sneakA.boundingBox;
    let bboxB = sneakB.boundingBox;

    let (aX, aY, _, _) = Revery.Math.BoundingBox2d.getBounds(bboxA);
    let (bX, bY, _, _) = Revery.Math.BoundingBox2d.getBounds(bboxB);

    aX -. bX +. (aY -. bY) |> int_of_float;
  };

  let allSneaks = sneaksToAdd |> List.sort(sort) |> List.mapi(toSneakInfo);
  let filteredSneaks = allSneaks;

  {...sneaks, allSneaks, filteredSneaks};
};

let getFiltered = ({filteredSneaks, _}) => filteredSneaks;

// REGISTRY
module Registry = {
  type sneakInfo = {
    node: ref(option(Revery.UI.node)),
    callback: unit => unit,
  };

  module MutableState = {
    let singleton = ref([]);
  };

  let register = (node: ref(option(Revery.UI.node)), callback) => {
    MutableState.singleton := [{node, callback}, ...MutableState.singleton^];
  };

  let unregister = (node: ref(option(Revery.UI.node))) => {
    let filter = sneakInfo => sneakInfo.node !== node;
    MutableState.singleton := List.filter(filter, MutableState.singleton^);
  };

  let getSneaks = () => {
    MutableState.singleton^
    |> List.filter_map(item => {
         switch (item.node^) {
         | Some(node) => Some((node, item.callback))
         | None => None
         }
       })
    |> List.map(((node, callback)) => {
         {callback, boundingBox: node#getBoundingBox()}
       });
  };
};
