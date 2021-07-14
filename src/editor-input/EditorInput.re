module Key = Key;
module KeyCandidate = KeyCandidate;
module Modifiers = Modifiers;
module Matcher = Matcher;
module KeyPress = KeyPress;
module PhysicalKey = PhysicalKey;
module SpecialKey = SpecialKey;

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
    (
      ~allowRecursive: bool,
      Matcher.t,
      context => bool,
      list(KeyPress.t),
      t
    ) =>
    (t, uniqueId);

  let disable: t => t;
  let enable: t => t;

  type effect =
    | Execute(command)
    | Text(string)
    | Unhandled({
        key: KeyCandidate.t,
        isProducedByRemap: bool,
      })
    | RemapRecursionLimitHit;

  let keyDown:
    (
      ~leaderKey: option(PhysicalKey.t)=?,
      ~context: context,
      ~scancode: int,
      ~key: KeyCandidate.t,
      t
    ) =>
    (t, list(effect));
  let text: (~text: string, t) => (t, list(effect));
  let keyUp:
    (
      ~leaderKey: option(PhysicalKey.t)=?,
      ~context: context,
      ~scancode: int,
      t
    ) =>
    (t, list(effect));

  let timeout: (~context: context, t) => (t, list(effect));

  let candidates:
    (~leaderKey: option(PhysicalKey.t), ~context: context, t) =>
    list((Matcher.t, command));

  let consumedKeys: t => list(KeyCandidate.t);

  let remove: (uniqueId, t) => t;

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
    | Unhandled({
        key: KeyCandidate.t,
        isProducedByRemap: bool,
      })
    | RemapRecursionLimitHit;

  type action =
    | Dispatch(command)
    | Remap({
        allowRecursive: bool,
        keys: list(KeyPress.t),
      });

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
    | Down(keyDownId, KeyCandidate.t)
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
    enabled: bool,
  };

  let enable = model => {...model, enabled: true};
  let disable = model => {...model, enabled: false};

  let count = ({bindings, _}) => List.length(bindings);

  let concat = (first, second) => {
    suppressText: false,
    bindings: first.bindings @ second.bindings,
    revKeys: [],
    text: [],
    pressedScancodes: IntSet.empty,
    enabled: first.enabled && second.enabled,
  };

  let consumedKeys = ({revKeys, _}) => {
    revKeys
    |> List.rev
    |> List.filter_map(
         fun
         | Down(_id, key) => Some(key)
         | AllKeysReleased => None,
       );
  };

  let keyMatches =
      (~leaderKey: option(PhysicalKey.t), keyMatcher, key: gesture) => {
    switch (key) {
    | Down(_id, keyCandidates) =>
      switch (keyMatcher) {
      | KeyPress.SpecialKey(Leader) =>
        switch (leaderKey) {
        | None =>
          KeyCandidate.exists(
            ~f=key => KeyPress.equals(KeyPress.SpecialKey(Leader), key),
            keyCandidates,
          )
        | Some(leaderKey) =>
          KeyCandidate.exists(
            ~f=key => KeyPress.equals(KeyPress.PhysicalKey(leaderKey), key),
            keyCandidates,
          )
        }
      | keyPress =>
        KeyCandidate.exists(
          ~f=key => KeyPress.equals(key, keyPress),
          keyCandidates,
        )
      }
    | _ => false
    };
  };

  let remove = (uniqueId, model) => {
    ...model,
    bindings: model.bindings |> List.filter(binding => binding.id != uniqueId),
  };

  let applyKeyToBinding = (~leaderKey, ~context, key, binding: binding) =>
    if (!binding.enabled(context)) {
      None;
    } else {
      switch (binding.matcher) {
      | Unmatched(Matcher.AllKeysReleased) when key == AllKeysReleased =>
        Some({...binding, matcher: Matched})
      | Unmatched(Matcher.Sequence([hd, ...tail]))
          when keyMatches(~leaderKey, hd, key) =>
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

  let applyKeyToBindings = (~leaderKey, ~context, key, bindings) => {
    List.filter_map(applyKeyToBinding(~leaderKey, ~context, key), bindings);
  };

  let removeRemaps = bindings => {
    bindings
    |> List.filter_map(binding =>
         switch (binding.action) {
         | Remap(_) => None
         | Dispatch(_) => Some(binding)
         }
       );
  };

  let applyKeysToBindings =
      (~allowRemaps, ~leaderKey, ~context, keys, bindings) => {
    // Filter out remaps if we're not allowing them
    let bindings =
      if (!allowRemaps) {
        bindings |> removeRemaps;
      } else {
        bindings;
      };

    let keyPresses =
      keys
      |> List.filter_map(
           fun
           | AllKeysReleased => None
           | Down(id, key) => Some(Down(id, key)),
         );

    let bindingsWithKeyApplied =
      keyPresses
      |> List.fold_left(
           (acc, curr) => {
             applyKeyToBindings(~leaderKey, ~context, curr, acc)
           },
           bindings,
         );

    bindingsWithKeyApplied;
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

  let addMapping = (~allowRecursive, matcher, enabled, keys, keyBindings) => {
    let {bindings, _} = keyBindings;
    let id = UniqueId.get();
    let bindings = [
      {
        id,
        matcher: Unmatched(matcher),
        action: Remap({allowRecursive, keys}),
        enabled,
      },
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

  let getEffectFromGesture = (~isRemap, gesture, bindings) => {
    switch (gesture) {
    | Down(id, keyPress) =>
      switch (getTextFromKeyId(id, bindings)) {
      | None => Some(Unhandled({key: keyPress, isProducedByRemap: isRemap}))
      | Some(text) => Some(Text(text))
      }
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
  let popKey = (~isRemap, bindings) => {
    switch (bindings.revKeys) {
    | [] => (bindings, None)
    | [latestKey, ...prevKeys] =>
      let eff = getEffectFromGesture(~isRemap, latestKey, bindings);
      let text = getTextNotMatchingKeys(bindings.text, [latestKey]);

      ({...bindings, revKeys: prevKeys, text}, eff);
    };
  };

  // Pop keys for matcher, and ignore all effects
  let useUpKeysForBinding =
      (
        ~allowRemaps,
        ~isRemap,
        ~leaderKey,
        ~context,
        ~bindingId,
        initialBindings,
      ) => {
    let rec loop = bindings =>
      // Popped all the way
      if (bindings.revKeys == []) {
        bindings;
      } else {
        let candidateBindings =
          applyKeysToBindings(
            ~allowRemaps,
            ~leaderKey,
            ~context,
            bindings.revKeys |> List.rev,
            bindings.bindings,
          );

        if (List.exists(
              binding => {binding.id == bindingId},
              candidateBindings,
            )) {
          let (bindings', _ignoredEffects) = popKey(~isRemap, bindings);
          loop(bindings');
        } else {
          bindings;
        };
      };
    loop(initialBindings);
  };

  let popAll = (~isRemap, ~revEffects, initialBindings) => {
    let rec loop = (~revEffects, bindings) =>
      if (bindings.revKeys == []) {
        (bindings, revEffects);
      } else {
        let (bindings', maybeEffect) = popKey(~isRemap, bindings);
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
    enabled: true,
  };

  let candidates = (~leaderKey, ~context, {revKeys, enabled, bindings, _}) =>
    if (!enabled) {
      [];
    } else {
      applyKeysToBindings(
        ~allowRemaps=true,
        ~leaderKey,
        ~context,
        revKeys |> List.rev,
        bindings,
      )
      |> List.filter_map(({matcher, action, enabled, _}: binding) => {
           switch (action) {
           | Remap(_) => None // TODO
           | Dispatch(command) =>
             switch (matcher) {
             | Matched => None
             | Unmatched(matchers) when enabled(context) =>
               Some((matchers, command))
             | _ => None
             }
           }
         });
    };

  let rec handleKeyCore =
          (
            ~isRemap=false,
            ~allowRemaps,
            ~leaderKey,
            ~recursionDepth=0,
            ~context,
            gesture,
            bindings,
          ) =>
    if (!bindings.enabled) {
      let eff =
        switch (gesture) {
        | Down(_id, key) => [
            Unhandled({key, isProducedByRemap: recursionDepth > 0}),
          ]
        | AllKeysReleased => []
        };
      (bindings, eff);
    } else if (recursionDepth > Constants.maxRecursiveDepth) {
      // Hit the maximum remap recursion depth - just bail on this key.
      let eff =
        switch (gesture) {
        | Down(_id, key) => [
            Unhandled({key, isProducedByRemap: recursionDepth > 0}),
            RemapRecursionLimitHit,
          ]
        | AllKeysReleased => [RemapRecursionLimitHit]
        };
      (empty, eff);
    } else {
      let revKeys = [gesture, ...bindings.revKeys];

      let candidateBindings =
        applyKeysToBindings(
          ~allowRemaps,
          ~leaderKey,
          ~context,
          revKeys |> List.rev,
          bindings.bindings,
        );

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
            useUpKeysForBinding(
              ~allowRemaps,
              ~isRemap,
              ~leaderKey,
              ~context,
              ~bindingId=binding.id,
              bindings,
            );
          let text = getTextNotMatchingKeys(bindings'.text, revKeys);
          switch (binding.action) {
          | Dispatch(command) => (
              {...bindings', text, suppressText: true},
              [Execute(command)],
            )

          | Remap(_) =>
            // Let flush handle the remap action
            flush(
              ~allowRemaps,
              ~isRemap=true,
              ~leaderKey,
              ~recursionDepth,
              ~context,
              {...bindings, suppressText: true, revKeys},
            )
          };

        | None =>
          flush(
            ~allowRemaps,
            ~isRemap,
            ~leaderKey,
            ~recursionDepth,
            ~context,
            {...bindings, revKeys},
          )
        };
      };
    }

  and runRemappedKeys =
      (
        ~allowRecursive,
        ~leaderKey,
        ~recursionDepth,
        ~context,
        ~keys: list(KeyCandidate.t),
        bindings,
      ) => {
    let (bindings', effects') =
      keys
      |> List.fold_left(
           (acc, key: KeyCandidate.t) => {
             let gesture = Down(KeyDownId.get(), key);
             let (bindings, effs) = acc;
             let (bindings', effects') =
               handleKeyCore(
                 ~allowRemaps=allowRecursive,
                 ~isRemap=true,
                 ~leaderKey,
                 ~recursionDepth,
                 ~context,
                 gesture,
                 bindings,
               );

             (bindings', effs @ effects');
           },
           (bindings, []),
         );
    (bindings', effects');
  }

  and flush =
      (
        ~allowRemaps,
        ~isRemap,
        ~leaderKey,
        ~recursionDepth,
        ~context,
        initialBindings,
      ) => {
    let rec loop = (~revEffects: list(effect), bindings) =>
      if (bindings.revKeys == []) {
        (bindings, revEffects);
      } else {
        let candidateBindings =
          applyKeysToBindings(
            ~allowRemaps,
            ~leaderKey,
            ~context,
            bindings.revKeys |> List.rev,
            bindings.bindings,
          );

        let readyBindings = getReadyBindings(candidateBindings);

        switch (List.nth_opt(readyBindings, 0)) {
        | None =>
          // No binding ready... pop the key, see if anything else is ready.
          let (bindings', maybeEff) = popKey(~isRemap, bindings);
          let effs = Option.to_list(maybeEff);
          loop(~revEffects=effs @ revEffects, bindings');
        | Some(binding) =>
          switch (binding.action) {
          | Remap({keys, allowRecursive}) =>
            // Use up keys for the bindings
            let rewindBindings =
              useUpKeysForBinding(
                ~allowRemaps,
                ~isRemap,
                ~leaderKey,
                ~context,
                ~bindingId=binding.id,
                bindings,
              );

            let candidates = keys |> List.map(KeyCandidate.ofKeyPress);

            // Run all the new keys
            runRemappedKeys(
              ~allowRecursive=allowRemaps && allowRecursive,
              ~leaderKey,
              ~recursionDepth=recursionDepth + 1,
              ~context,
              ~keys=candidates,
              {...rewindBindings, suppressText: true},
            );
          | Dispatch(command) =>
            let revEffects = [Execute(command), ...revEffects];
            // Use up keys related to this binding
            let rewindBindings =
              useUpKeysForBinding(
                ~allowRemaps,
                ~isRemap,
                ~leaderKey,
                ~context,
                ~bindingId=binding.id,
                bindings,
              );
            // And then anything else past it - treat as unhandled
            popAll(
              ~isRemap,
              ~revEffects,
              {...rewindBindings, suppressText: true},
            );
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

  let keyDown = (~leaderKey=None, ~context, ~scancode, ~key, bindings) => {
    let id = KeyDownId.get();
    let pressedScancodes = IntSet.add(scancode, bindings.pressedScancodes);

    // #2778 - Don't allow modifiers to interrupt candidate bindings
    if (!KeyCandidate.isModifier(key)) {
      let (model, effects) =
        handleKeyCore(
          ~allowRemaps=true,
          ~leaderKey,
          ~context,
          Down(id, key),
          {...bindings, pressedScancodes},
        );

      let isUnhandled =
        switch (effects) {
        | [Unhandled({isProducedByRemap: false, _})] => true
        | _ => false
        };

      let model' =
        if (isUnhandled) {
          {...model, suppressText: false};
        } else {
          model;
        };
      (model', effects);
    } else {
      ({...bindings, pressedScancodes}, []);
    };
  };

  let text = (~text, bindings) =>
    // The last key down participated in a binding,
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

  let getEffectsForReleaseBindings = (~leaderKey, ~context, bindings) => {
    let releaseBindings =
      bindings.bindings
      |> List.filter_map(
           applyKeyToBinding(~leaderKey, ~context, AllKeysReleased),
         );

    let rec loop = bindings =>
      switch (bindings) {
      // For '<release>', only care about dispatch actions
      | [{action: Dispatch(command), _}, ..._] => [Execute(command)]

      | [_hd, ...tail] => loop(tail)
      | [] => []
      };

    loop(releaseBindings);
  };

  let keyUp = (~leaderKey=None, ~context, ~scancode, bindings) => {
    let pressedScancodes = IntSet.remove(scancode, bindings.pressedScancodes);

    let bindings = {...bindings, suppressText: false, pressedScancodes};

    // If everything has been released, fire an [AllKeysReleased] event,
    // in case anything is listening for it.
    let initialEffects =
      if (IntSet.is_empty(pressedScancodes)) {
        getEffectsForReleaseBindings(~leaderKey, ~context, bindings);
      } else {
        [];
      };

    (bindings, initialEffects);
  };

  let timeout = (~context, bindings) => {
    // Timeout is like pressing a non-existent key - it should flush all existing bindings

    // let noopKey = KeyCandidate.ofList([]);
    // let (bindings', keyDownEffects) = keyDown(~context, ~key=noopKey, ~scancode=-1, bindings);
    // let (bindings'', keyUpEffects) = keyUp(~context, ~scancode=-1, bindings');
    let (bindings'', effects) =
      flush(
        ~allowRemaps=false,
        ~isRemap=false,
        ~leaderKey=None,
        ~recursionDepth=0,
        ~context,
        bindings,
      );

    (bindings'', effects);
  };
};
