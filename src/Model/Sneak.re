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
  | NoneAvailable
  | Executed([@opaque] sneak)
  | Discovered([@opaque] list(sneakInfo))
  | KeyboardInput(string);

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

let getTextHighlight = (text: string, model: t) => {
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
    Oni_Core.Utility.StringUtil.startsWith(~prefix, sneak.id);
  };

  let applyFilter = (sneaks: t) =>
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

let refine = (characterToAdd: string, sneaks: t) => {
  let characterToAdd = String.uppercase_ascii(characterToAdd);

  let (prefix, _) =
    InputModel.handleInput(
      ~text=sneaks.prefix,
      ~cursorPosition=String.length(sneaks.prefix),
      characterToAdd,
    );

  {...sneaks, prefix} |> Internal.applyFilter;
};

let add = (sneaksToAdd: list(sneakInfo), sneaks: t) => {
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

let getFiltered = (sneaks: t) => sneaks.filteredSneaks;
