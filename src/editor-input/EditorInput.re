module Key = Key;
module Modifiers = Modifiers;
module Matcher = Matcher;
module KeyPress = KeyPress;

module IntSet =
  Set.Make({
    let compare = Stdlib.compare;
    type t = int;
  });

module type Input = {
  type command;
  type context;

  type t;

  type uniqueId;

  let addBinding: (Matcher.t, context => bool, command, t) => (t, uniqueId);
  let addMapping:
    (Matcher.t, context => bool, list(KeyPress.t), t) => (t, uniqueId);

  type effect =
    | Execute(command)
    | Text(string)
    | Unhandled(KeyPress.t);

  let keyDown: (~context: context, ~key: KeyPress.t, t) => (t, list(effect));
  let text: (~text: string, t) => (t, list(effect));
  let keyUp: (~context: context, ~key: KeyPress.t, t) => (t, list(effect));
  let flush: (~context: context, t) => (t, list(effect));

  let isPending: t => bool;

  let count: t => int;

  let concat: (t, t) => t;

  let empty: t;
};

module UniqueId = {
  let nextId = ref(0);

  let get = () => {
    let id = nextId^;
    incr(nextId);
    id;
  };
};

module KeyDownId = {
  let nextId = ref(0);

  let get = () => {
    let id = nextId^;
    incr(nextId);
    id;
  };
};

module Make = (Config: {
                 type command;
                 type context;
               }) => {
  type command = Config.command;
  type context = Config.context;

  type effect =
    | Execute(command)
    | Text(string)
    | Unhandled(KeyPress.t);

  type action =
    | Dispatch(command)
    | Remap(list(KeyPress.t));

  type matchState =
    | Matched
    | Unmatched(Matcher.t);

  type binding = {
    id: int,
    matcher: matchState,
    action,
    enabled: context => bool,
  };

  type uniqueId = int;

  type keyDownId = int;

  type gesture =
    | Down(keyDownId, KeyPress.t)
    | Up(KeyPress.t)
    | AllKeysReleased;

  type textEntry = {
    keyDownId,
    text: string,
  };

  type t = {
    lastDownKey: option(keyDownId),
    suppressText: bool,
    bindings: list(binding),
    text: list(textEntry),
    keys: list(gesture),
    pressedScancodes: IntSet.t,
  };

  let count = ({bindings, _}) => List.length(bindings);

  let concat = (first, second) => {
    suppressText: false,
    lastDownKey: None,
    bindings: first.bindings @ second.bindings,
    keys: [],
    text: [],
    pressedScancodes: IntSet.empty,
  };

  let keyMatches = (keyMatcher, key: gesture) => {
    Matcher.(
      {
        switch (keyMatcher, key) {
        | (Keydown(Scancode(scancode, mods)), Down(_id, key)) =>
          key.scancode == scancode && Modifiers.equals(mods, key.modifiers)
        | (Keydown(Keycode(keycode, mods)), Down(_id, key)) =>
          key.keycode == keycode && Modifiers.equals(mods, key.modifiers)
        | (Keyup(Scancode(scancode, mods)), Up(key)) =>
          key.scancode == scancode && Modifiers.equals(mods, key.modifiers)
        | (Keyup(Keycode(keycode, mods)), Up(key)) =>
          key.keycode == keycode && Modifiers.equals(mods, key.modifiers)
        | _ => false
        };
      }
    );
  };

  let applyKeyToBinding = (~context, key, binding) =>
    if (!binding.enabled(context)) {
      None;
    } else {
      switch (binding.matcher) {
      | Unmatched(Matcher.AllKeysReleased) when key == AllKeysReleased =>
        Some({...binding, matcher: Matched})
      | Unmatched(Matcher.Sequence([hd, ...tail])) when keyMatches(hd, key) =>
        if (tail == []) {
          // If the sequence is fully exercise, we're matched!
          Some({
            ...binding,
            matcher: Matched,
          });
        } else {
          // Otherwise, pull the matched key off, and leave the remainder of keys to match
          Some({
            ...binding,
            matcher: Unmatched(Matcher.Sequence(tail)),
          });
        }
      | _ => None
      };
    };

  let applyKeyToBindings = (~context, key, bindings) => {
    List.filter_map(applyKeyToBinding(~context, key), bindings);
  };

  let applyKeysToBindings = (~context, keys, bindings) => {
    let bindingsWithKeyUp =
      keys
      |> List.fold_left(
           (acc, curr) => {applyKeyToBindings(~context, curr, acc)},
           bindings,
         );

    let consumedBindings =
      bindingsWithKeyUp
      |> List.fold_left(
           (acc, curr) => {
             Hashtbl.add(acc, curr.id, true);
             acc;
           },
           Hashtbl.create(16),
         );

    let unusedBindings =
      bindings
      |> List.filter(binding =>
           Stdlib.Option.is_none(
             Hashtbl.find_opt(consumedBindings, binding.id),
           )
         );

    let keysWithoutUps =
      keys
      |> List.filter_map(
           fun
           | AllKeysReleased => None
           | Down(id, key) => Some(Down(id, key))
           | Up(_key) => None,
         );

    let bindingsWithoutUpKey =
      keysWithoutUps
      |> List.fold_left(
           (acc, curr) => {applyKeyToBindings(~context, curr, acc)},
           unusedBindings,
         );

    bindingsWithKeyUp @ bindingsWithoutUpKey;
  };

  let addBinding = (matcher, enabled, command, keyBindings) => {
    let {bindings, _} = keyBindings;
    let id = UniqueId.get();
    let bindings = [
      {id, matcher: Unmatched(matcher), action: Dispatch(command), enabled},
      ...bindings,
    ];

    let newBindings = {...keyBindings, bindings};
    (newBindings, id);
  };

  let addMapping = (matcher, enabled, keys, keyBindings) => {
    let {bindings, _} = keyBindings;
    let id = UniqueId.get();
    let bindings = [
      {id, matcher: Unmatched(matcher), action: Remap(keys), enabled},
      ...bindings,
    ];

    let newBindings = {...keyBindings, bindings};
    (newBindings, id);
  };

  let reset = (~keys=[], ~text=[], bindings) => {
    ...bindings,
    lastDownKey: None,
    text,
    keys,
  };

  let getReadyBindings = bindings => {
    let filter = binding => binding.matcher == Matched;

    bindings |> List.filter(filter);
  };

  let keyIdsToHashtable = (keys: list(gesture)) => {
    let ret = Hashtbl.create(64);

    keys
    |> List.iter(curr => {
         switch (curr) {
         | AllKeysReleased => ()
         | Up(_key) => ()
         | Down(id, _key) => Hashtbl.add(ret, id, true)
         }
       });

    ret;
  };

  let getTextMatchingKeys = (text, keys) => {
    let hash = keyIdsToHashtable(keys);

    text |> List.filter(textEntry => Hashtbl.mem(hash, textEntry.keyDownId));
  };

  let getTextNotMatchingKeys = (text, keys) => {
    let hash = keyIdsToHashtable(keys);

    text |> List.filter(textEntry => !Hashtbl.mem(hash, textEntry.keyDownId));
  };

  let isRemap = ({action, _}) =>
    switch (action) {
    | Dispatch(_) => false
    | Remap(_) => true
    };

  module Constants = {
    let maxRecursiveDepth = 10;
  };

  let flush = (~context, bindings) => {
    let allKeys = bindings.keys;

    let rec loop =
            (
              ~flush,
              revKeys,
              remainingText: list(textEntry),
              remainingKeys,
              effects,
              iterationCount,
            ) => {
      let candidateBindings =
        applyKeysToBindings(~context, revKeys |> List.rev, bindings.bindings);

      // If we've hit the recursion limit for remaps... filter out remaps
      let candidateBindings =
        if (iterationCount > Constants.maxRecursiveDepth) {
          candidateBindings |> List.filter(binding => !isRemap(binding));
        } else {
          candidateBindings;
        };

      let readyBindings = getReadyBindings(candidateBindings);
      let readyBindingCount = List.length(readyBindings);
      let candidateBindingCount = List.length(candidateBindings);

      let potentialBindingCount = candidateBindingCount - readyBindingCount;

      switch (List.nth_opt(readyBindings, 0)) {
      | Some(binding) =>
        if (flush || potentialBindingCount == 0) {
          // Filter out any 'text' entries that are associated with the keys for this finding
          let remainingText = getTextNotMatchingKeys(remainingText, revKeys);

          switch (binding.action) {
          | Dispatch(command) => (
              remainingKeys,
              remainingText,
              [Execute(command), ...effects],
            )
          | Remap(keys) =>
            let newKeys =
              keys |> List.map(k => Down(KeyDownId.get(), k)) |> List.rev;
            loop(
              ~flush,
              newKeys,
              remainingText,
              remainingKeys,
              effects,
              iterationCount + 1,
            );
          };
        } else {
          (List.append(revKeys, remainingKeys), remainingText, effects);
        }
      // Queue keys -
      | None when potentialBindingCount > 0 => (
          // We have more bindings available, so just stash our keys and quit
          List.append(revKeys, remainingKeys),
          remainingText,
          effects,
        )
      // No candidate bindings... try removing a key and processing bindings
      | None =>
        switch (revKeys) {
        | [] =>
          // No keys left, we're done here
          (remainingKeys, remainingText, effects)
        | [Down(keyDownId, latestKey)] =>
          let textEffects =
            remainingText
            |> List.filter(textEntry => textEntry.keyDownId == keyDownId)
            |> List.map((textEntry: textEntry) => Text(textEntry.text));

          let remainingText =
            remainingText
            |> List.filter(textEntry => textEntry.keyDownId != keyDownId);

          // At the last key... if we got here, we couldn't find any match for this key
          (
            [],
            remainingText,
            [Unhandled(latestKey)] @ textEffects @ effects,
          );
        | [Up(_latestKey)] =>
          // At the last key... if we got here, we couldn't find any match for this key
          ([], remainingText, effects)
        | [latestKey, ...otherKeys] =>
          // Try a subset of keys
          loop(
            ~flush,
            otherKeys,
            remainingText,
            [latestKey, ...remainingKeys],
            effects,
            iterationCount,
          )
        }
      };
    };

    let (remainingKeys, remainingText, effects) =
      loop(~flush=true, allKeys, bindings.text, [], [], 0);

    let (remainingKeys, remainingText, effects) =
      loop(~flush=false, remainingKeys, remainingText, [], effects, 0);

    let keys = remainingKeys;

    // The text used for the commands was filtered out, so any now-unmatched
    // text is unhandled
    let unhandledText = getTextNotMatchingKeys(remainingText, keys);
    let currentText = getTextMatchingKeys(remainingText, keys);

    let textEffects =
      unhandledText
      |> List.map((textEntry: textEntry) => Text(textEntry.text));

    let text = currentText;
    (reset(~keys, ~text, bindings), textEffects @ effects);
  };

  let handleKeyCore = (~context, gesture, bindings) => {
    let originalKeys = bindings.keys;
    let keys = [gesture, ...bindings.keys];

    let candidateBindings =
      applyKeysToBindings(~context, keys |> List.rev, bindings.bindings);

    let readyBindings = getReadyBindings(candidateBindings);
    let readyBindingCount = List.length(readyBindings);
    let candidateBindingCount = List.length(candidateBindings);

    let potentialBindingCount = candidateBindingCount - readyBindingCount;

    if (potentialBindingCount > 0) {
      ({...bindings, keys}, []);
    } else {
      switch (List.nth_opt(readyBindings, 0)) {
      | Some(binding) =>
        let text = getTextNotMatchingKeys(bindings.text, keys);
        switch (binding.action) {
        | Dispatch(command) => (
            reset({...bindings, suppressText: true, text}),
            [Execute(command)],
          )
        | Remap(remappedKeys) =>
          let keys =
            List.append(
              originalKeys,
              List.map(k => Down(KeyDownId.get(), k), remappedKeys),
            );
          flush(~context, {...bindings, suppressText: true, text, keys});
        };
      | None => flush(~context, {...bindings, keys})
      };
    };
  };

  let isPending = ({keys, _}) => keys != [];

  let keyDown = (~context, ~key, bindings) => {
    let id = KeyDownId.get();
    handleKeyCore(
      ~context,
      Down(id, key),
      {
        ...bindings,
        pressedScancodes: IntSet.add(key.scancode, bindings.pressedScancodes),
        lastDownKey: Some(id),
      },
    );
  };

  let text = (~text, bindings) =>
    // The last key down participating in binding,
    // so we'll ignore text until we get a keyup
    if (bindings.suppressText) {
      (bindings, []);
    } else {
      switch (bindings.lastDownKey) {
      // If there is a pending key, hold on to the text input
      // until the gesture is completed
      | Some(keyDownId) => (
          {...bindings, text: [{keyDownId, text}, ...bindings.text]},
          [],
        )
      // Otherwise, just dispatch the Text event
      | None => (bindings, [Text(text)])
      };
    };

  let getEffectsForReleaseBindings = (~context, bindings) => {
    let releaseBindings =
      bindings.bindings
      |> List.filter_map(applyKeyToBinding(~context, AllKeysReleased));

    let rec loop = bindings =>
      switch (bindings) {
      // For '<release>', only care about dispatch actions
      | [{action: Dispatch(command), _}, ..._] => [Execute(command)]

      | [_hd, ...tail] => loop(tail)
      | [] => []
      };

    loop(releaseBindings);
  };

  let keyUp = (~context, ~key, bindings) => {
    let pressedScancodes =
      IntSet.remove(KeyPress.(key.scancode), bindings.pressedScancodes);
    let bindings = {...bindings, suppressText: false, pressedScancodes};

    // If everything has been released, fire an [AllKeysReleased] event,
    // in case anything is listening for it.
    let initialEffects =
      if (IntSet.is_empty(pressedScancodes)) {
        getEffectsForReleaseBindings(~context, bindings);
      } else {
        [];
      };

    let (bindings, effects) = handleKeyCore(~context, Up(key), bindings);

    (bindings, effects @ initialEffects);
  };

  let empty = {
    suppressText: false,
    text: [],
    lastDownKey: None,
    bindings: [],
    keys: [],
    pressedScancodes: IntSet.empty,
  };
};
