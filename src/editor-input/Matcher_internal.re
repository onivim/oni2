[@deriving show]
type modifier =
  | Control
  | Shift
  | Alt
  | Super;

[@deriving show]
type keyPress =
  | UnmatchedString(string)
  | Physical(Key.t)
  | Special(SpecialKey.t);

[@deriving show]
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
      | [Super, ...tail] => loop(Modifiers.{...mods, super: true}, tail)
      };

    loop(Modifiers.none, modList);
  };
};
