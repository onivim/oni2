type modifier =
  | Control
  | Shift
  | Alt
  | Meta;

type keyPress =
  | Physical(Key.t)
  | Special(SpecialKey.t);

type keyMatcher = (keyPress, list(modifier));

type t =
  | Sequence(list(keyMatcher))
  | AllKeysReleased;

type keyList = list(keyMatcher);

module Helpers = {
  let internalModsToMods = modList => {
    let rec loop = (mods, modList) =>
      switch (modList) {
      | [] => mods
      | [Control, ...tail] => loop(Modifiers.{...mods, control: true}, tail)
      | [Shift, ...tail] => loop(Modifiers.{...mods, shift: true}, tail)
      | [Alt, ...tail] => loop(Modifiers.{...mods, alt: true}, tail)
      | [Meta, ...tail] => loop(Modifiers.{...mods, meta: true}, tail)
      };

    loop(Modifiers.none, modList);
  };
};
