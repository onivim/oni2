// TODO: Functorize
type payload = string;
type context = bool;

module Modifiers = {
  type t = {
    control: bool,
    alt: bool,
    shift: bool,
    meta: bool,
  };

  let create = (~control, ~alt, ~shift, ~meta) => {
    control,
    alt,
    shift,
    meta,
  };

  let none = {control: false, alt: false, shift: false, meta: false};

  let equals = (_, _) => true;
};

type key = {
  scancode: int,
  keycode: int,
  modifiers: Modifiers.t,
  text: string,
};

type effects =
  | Execute(payload)
  | Unhandled(key);

type keyMatcher =
  | Scancode(int, Modifiers.t)
  | Keycode(int, Modifiers.t);

type sequence = list(keyMatcher);

type binding = {
  id: int,
  sequence,
  payload,
  enabled: context => bool,
};

type t = {
  nextId: int,
  allBindings: list(binding),
  keys: list(key),
};

let keyMatches = (keyMatcher, key) => {
  switch (keyMatcher) {
  | Scancode(scancode, mods) =>
    key.scancode == scancode && Modifiers.equals(mods, key.modifiers)
  | Keycode(keycode, mods) =>
    key.keycode == keycode && Modifiers.equals(mods, key.modifiers)
  };
};

let applyKeyToBinding = (key, binding) => {
  switch (binding.sequence) {
  | [hd, ...tail] when keyMatches(hd, key) =>
    Some({...binding, sequence: tail})
  | [] => None
  | _ => None
  };
};

let applyKeyToBindings = (key, bindings) => {
  List.filter_map(applyKeyToBinding(key), bindings);
};

let applyKeysToBindings = (keys, bindings) => {
  List.fold_left(
    (acc, curr) => {applyKeyToBindings(curr, acc)},
    bindings,
    keys,
  );
};

let addBinding = (sequence, enabled, payload, bindings) => {
  let {nextId, allBindings, _} = bindings;
  let allBindings = [
    {id: nextId, sequence, payload, enabled},
    ...allBindings,
  ];

  let newBindings = {...bindings, allBindings, nextId: nextId + 1};
  (newBindings, nextId);
};

let reset = (~keys=[], bindings) => {...bindings, keys};

let getReadyBindings = bindings => {
  let filter = binding => binding.sequence == [];

  bindings |> List.filter(filter);
};

let flush = bindings => {
  //let allBindings = bindings.allBindings;
  let allKeys = bindings.keys;

  let rec loop = (flush, revKeys, remainingKeys, effects) => {

    let candidateBindings =
      applyKeysToBindings(revKeys |> List.rev, bindings.allBindings);
    let readyBindings = getReadyBindings(candidateBindings);
    let readyBindingCount = List.length(readyBindings);
    let candidateBindingCount = List.length(candidateBindings);

    let potentialBindingCount = candidateBindingCount - readyBindingCount;

    switch (List.nth_opt(readyBindings, 0)) {
    | Some(binding) =>
      if (flush || potentialBindingCount == 0) {
        (remainingKeys, [Execute(binding.payload), ...effects]);
      } else {
        (List.append(revKeys, remainingKeys), effects);
      }
    // Queue keys -
    | None when potentialBindingCount > 0 => (
        // We have more bindings available, so just stash our keys and quit
        List.append(revKeys, remainingKeys),
        effects,
      )
    // No candidate bindings... try removing a key and processing bindings
    | None =>
      switch (revKeys) {
      | [] =>
        // No keys left, we're done here
        (remainingKeys, effects)
      | [latestKey] =>
        // At the last key... if we got here, we couldn't find any match for this key
        ([], [Unhandled(latestKey), ...effects])
      | [latestKey, ...otherKeys] =>
        // Try a subset of keys
        loop(flush, otherKeys, [latestKey, ...remainingKeys], effects)
      }
    // TODO
    };
  };

  let (remainingKeys, effects) = loop(true, allKeys, [], []);

  let (remainingKeys, effects) = loop(false, remainingKeys, [], effects);

  let keys = remainingKeys;
  (reset(~keys, bindings), effects);
};

let keyDown = (key, bindings) => {

  let keys = [key, ...bindings.keys];

  let candidateBindings =
    applyKeysToBindings(keys |> List.rev, bindings.allBindings);

  let readyBindings = getReadyBindings(candidateBindings);
  let readyBindingCount = List.length(readyBindings);
  let candidateBindingCount = List.length(candidateBindings);

  let potentialBindingCount = candidateBindingCount - readyBindingCount;

  if (potentialBindingCount > 0) {
    ({...bindings, keys}, []);
  } else {
    switch (List.nth_opt(readyBindings, 0)) {
    | Some(binding) =>
      (reset(bindings), [Execute(binding.payload)]);
    | None =>
      flush({...bindings, keys});
    };
  };
};

let keyUp = (_key, bindings) => (bindings, []);

let empty = {nextId: 0, allBindings: [], keys: []};
