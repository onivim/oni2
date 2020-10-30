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
    | Unhandled(KeyPress.t)
    | RemapRecursionLimitHit;

  let keyDown: (~context: context, ~key: KeyPress.t, t) => (t, list(effect));
  let text: (~text: string, t) => (t, list(effect));
  let keyUp: (~context: context, ~key: KeyPress.t, t) => (t, list(effect));

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
    | Unhandled(KeyPress.t)
    | RemapRecursionLimitHit;

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
    // The `suppressText` flag is used to handle the flow of input events:
    // - KeyDown
    // - Text (maybe)
    // - KeyUp
    // When a KeyDown is sent - there may or may not be a subsequent event.
    // If a KeyDown did not participate in a binding, we dispatch the Text
    // event preferably. However, if the KeyDown did participate in a binding,
    // we set the suppressText flag to prevent dispatching a corresponding
    // Text event in that case. Once there is a KeyUp - we reset that flag.
    suppressText: bool,
    bindings: list(binding),
    text: list(textEntry),
    // Keys stored in reverse order - most recent keys first
    revKeys: list(gesture),
    pressedScancodes: IntSet.t,
  };

  let count = ({bindings, _}) => List.length(bindings);

  let concat = (first, second) => {
    suppressText: false,
    bindings: first.bindings @ second.bindings,
    revKeys: [],
    text: [],
    pressedScancodes: IntSet.empty,
  };

  let keyMatches = (keyMatcher, key: gesture) => {
    switch (keyMatcher, key) {
    | (KeyPress.{keycode, modifiers, _}, Down(_id, key)) =>
      key.keycode == keycode && Modifiers.equals(modifiers, key.modifiers)
    | _ => false
    };
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

  let getTextFromKeyId = (keyId, {text, _}: t) => {
    text
    |> List.filter(textEntry => textEntry.keyDownId == keyId)
    |> (
      l =>
        List.nth_opt(l, 0)
        |> Option.map((textEntry: textEntry) => textEntry.text)
    );
  };

  let getEffectFromGesture = (gesture, bindings) => {
    switch (gesture) {
    | Down(id, keyPress) =>
      switch (getTextFromKeyId(id, bindings)) {
      | None => Some(Unhandled(keyPress))
      | Some(text) => Some(Text(text))
      }
    | Up(_)
    | AllKeysReleased => None
    };
  };

  let getTextNotMatchingKeys = (text, keys) => {
    let hash = keyIdsToHashtable(keys);

    text |> List.filter(textEntry => !Hashtbl.mem(hash, textEntry.keyDownId));
  };

  module Constants = {
    let maxRecursiveDepth = 10;
  };

  // Pop the latest key, and return the bindings w/o the key, and any effect
  let popKey = bindings => {
    switch (bindings.revKeys) {
    | [] => (bindings, None)
    | [latestKey, ...prevKeys] =>
      let eff = getEffectFromGesture(latestKey, bindings);
      let text = getTextNotMatchingKeys(bindings.text, [latestKey]);

      ({...bindings, revKeys: prevKeys, text}, eff);
    };
  };

  // Pop keys for matcher, and ignore all effects
  let useUpKeysForBinding = (~context, ~bindingId, initialBindings) => {
    let rec loop = bindings =>
      // Popped all the way
      if (bindings.revKeys == []) {
        bindings;
      } else {
        let candidateBindings =
          applyKeysToBindings(
            ~context,
            bindings.revKeys |> List.rev,
            bindings.bindings,
          );

        if (List.exists(
              binding => {binding.id == bindingId},
              candidateBindings,
            )) {
          let (bindings', _ignoredEffects) = popKey(bindings);
          loop(bindings');
        } else {
          bindings;
        };
      };
    loop(initialBindings);
  };

  let popAll = (~revEffects, initialBindings) => {
    let rec loop = (~revEffects, bindings) =>
      if (bindings.revKeys == []) {
        (bindings, revEffects);
      } else {
        let (bindings', maybeEffect) = popKey(bindings);
        let effs = Option.to_list(maybeEffect);
        loop(~revEffects=effs @ revEffects, bindings');
      };
    loop(~revEffects, initialBindings);
  };

  let empty = {
    suppressText: false,
    text: [],
    bindings: [],
    revKeys: [],
    pressedScancodes: IntSet.empty,
  };

  let rec handleKeyCore = (~recursionDepth=0, ~context, gesture, bindings) =>
    // Hit the maximum remap recursion depth - just bail on this key.
    if (recursionDepth > Constants.maxRecursiveDepth) {
      let eff =
        switch (gesture) {
        | Down(_id, key) => [Unhandled(key), RemapRecursionLimitHit]
        | Up(_) => [RemapRecursionLimitHit]
        | AllKeysReleased => [RemapRecursionLimitHit]
        };
      (empty, eff);
    } else {
      let revKeys = [gesture, ...bindings.revKeys];

      let candidateBindings =
        applyKeysToBindings(~context, revKeys |> List.rev, bindings.bindings);

      let readyBindings = getReadyBindings(candidateBindings);
      let readyBindingCount = List.length(readyBindings);
      let candidateBindingCount = List.length(candidateBindings);

      let potentialBindingCount = candidateBindingCount - readyBindingCount;

      // We've got more than one potential binding, so let's wait
      // and see what the user presses next.
      if (potentialBindingCount > 0) {
        ({...bindings, revKeys}, []);
      } else {
        // No 'potential' bindings - but is there one ready?
        switch (List.nth_opt(readyBindings, 0)) {
        // There is a matching, ready binding - let's run it.
        | Some(binding) =>
          let bindings' =
            useUpKeysForBinding(~context, ~bindingId=binding.id, bindings);
          let text = getTextNotMatchingKeys(bindings'.text, revKeys);
          switch (binding.action) {
          | Dispatch(command) => (
              {...bindings', text, suppressText: true},
              [Execute(command)],
            )

          | Remap(_) =>
            // Let flush handle the remap action
            flush(
              ~recursionDepth,
              ~context,
              {...bindings, suppressText: true, revKeys},
            )
          };

        | None => flush(~recursionDepth, ~context, {...bindings, revKeys})
        };
      };
    }

  and runRemappedKeys = (~recursionDepth, ~context, ~keys, bindings) => {
    let (bindings', effects') =
      keys
      |> List.fold_left(
           (acc, key) => {
             let gesture = Down(KeyDownId.get(), key);
             let (bindings, effs) = acc;
             let (bindings', effects') =
               handleKeyCore(~recursionDepth, ~context, gesture, bindings);

             (bindings', effects' @ effs);
           },
           (bindings, []),
         );
    (bindings', List.rev(effects'));
  }

  and flush = (~recursionDepth, ~context, initialBindings) => {
    let rec loop = (~revEffects: list(effect), bindings) =>
      if (bindings.revKeys == []) {
        (bindings, revEffects);
      } else {
        let candidateBindings =
          applyKeysToBindings(
            ~context,
            bindings.revKeys |> List.rev,
            bindings.bindings,
          );

        let readyBindings = getReadyBindings(candidateBindings);

        switch (List.nth_opt(readyBindings, 0)) {
        | None =>
          // No binding ready... pop the key, see if anything else is ready.
          let (bindings', maybeEff) = popKey(bindings);
          let effs = Option.to_list(maybeEff);
          loop(~revEffects=effs @ revEffects, bindings');
        | Some(binding) =>
          switch (binding.action) {
          | Remap(keys) =>
            // Use up keys for the bindings
            let rewindBindings =
              useUpKeysForBinding(~context, ~bindingId=binding.id, bindings);

            // Run all the new keys
            runRemappedKeys(
              ~recursionDepth=recursionDepth + 1,
              ~context,
              ~keys,
              {...rewindBindings, suppressText: true},
            );
          | Dispatch(command) =>
            let revEffects = [Execute(command), ...revEffects];
            // Use up keys related to this binding
            let rewindBindings =
              useUpKeysForBinding(~context, ~bindingId=binding.id, bindings);
            // And then anything else past it - treat as unhandled
            popAll(~revEffects, {...rewindBindings, suppressText: true});
          }
        };
      };

    let (outBindings, outEffects) = loop(~revEffects=[], initialBindings);
    (outBindings, outEffects);
  };

  let isPending = ({revKeys, _}) => revKeys != [];

  let lastDownKey = ({revKeys, _}) => {
    switch (revKeys) {
    | [Down(keyDownId, _), ..._] => Some(keyDownId)
    | _ => None
    };
  };

  let keyDown = (~context, ~key, bindings) => {
    let id = KeyDownId.get();
    handleKeyCore(
      ~context,
      Down(id, key),
      {
        ...bindings,
        pressedScancodes: IntSet.add(key.scancode, bindings.pressedScancodes),
      },
    );
  };

  let text = (~text, bindings) =>
    // The last key down participating in binding,
    // so we'll ignore text until we get a keyup
    if (bindings.suppressText) {
      (bindings, []);
    } else {
      switch (lastDownKey(bindings)) {
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
};
