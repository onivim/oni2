open TestFramework;
open EditorInput;

let aKeyScancode = 101;
let aKeyNoModifiers =
  KeyPress.physicalKey(
    ~key=Key.Character(Uchar.of_char('a')),
    ~modifiers=Modifiers.none,
  );

let ctrlAKey =
  KeyPress.physicalKey(
    ~key=Key.Character(Uchar.of_char('a')),
    ~modifiers=Modifiers.{...none, control: true},
  );

let bKeyScancode = 102;

let bKeyNoModifiers =
  KeyPress.physicalKey(
    ~key=Key.Character(Uchar.of_char('b')),
    ~modifiers=Modifiers.none,
  );

let ctrlBKey =
  KeyPress.physicalKey(
    ~key=Key.Character(Uchar.of_char('b')),
    ~modifiers=Modifiers.{...none, control: true},
  );

let cKeyScancode = 103;

let cKeyNoModifiers =
  KeyPress.physicalKey(
    ~key=Key.Character(Uchar.of_char('c')),
    ~modifiers=Modifiers.none,
  );

let ucharNoModifiers = uchar =>
  KeyPress.physicalKey(~key=Key.Character(uchar), ~modifiers=Modifiers.none);

let leaderKey = KeyPress.specialKey(SpecialKey.Leader);
let plugKey = KeyPress.specialKey(SpecialKey.Plug);

let leftControlKeyScancode = 901;
let leftControlKey =
  KeyPress.physicalKey(
    ~key=Key.LeftControl,
    ~modifiers=Modifiers.{...none, control: true},
  );

let candidate = keyPress => KeyCandidate.ofKeyPress(keyPress);

module Input =
  EditorInput.Make({
    type command = string;
    type context = bool;
  });

describe("EditorInput", ({describe, _}) => {
  describe("modifier keys", ({test, _}) => {
    test("#2778: Modifier keys shouldn't interrupt binding", ({expect, _}) => {
      let (bindings, _id) =
        Input.empty
        |> Input.addBinding(
             Sequence([ctrlAKey, ctrlBKey]),
             _ => true,
             "testCommand",
           );
      let (bindings, _effects) =
        Input.keyDown(
          ~context=true,
          ~scancode=leftControlKeyScancode,
          ~key=candidate(leftControlKey),
          bindings,
        );
      let (bindings, _effects) =
        Input.keyDown(
          ~context=true,
          ~scancode=aKeyScancode,
          ~key=candidate(ctrlAKey),
          bindings,
        );
      let (bindings, _effects) =
        Input.keyUp(
          ~context=true,
          ~scancode=leftControlKeyScancode,
          bindings,
        );
      // In #2778, this control-key press event in the midst of candidate bindings
      // caused the bindings to be reset - this is the important part of
      // exercising this case:
      let (bindings, _effects) =
        Input.keyDown(
          ~context=true,
          ~scancode=leftControlKeyScancode,
          ~key=candidate(leftControlKey),
          bindings,
        );
      let (_bindings, effects) =
        Input.keyDown(
          ~context=true,
          ~scancode=bKeyScancode,
          ~key=candidate(ctrlBKey),
          bindings,
        );

      expect.equal(effects, [Execute("testCommand")]);
    })
  });
  describe("special keys", ({test, _}) => {
    test("special keys can participate in remap", ({expect, _}) => {
      // Add a <Plug>a -> "commandA" mapping
      let (bindings, _id) =
        Input.empty
        |> Input.addBinding(
             Sequence([plugKey, aKeyNoModifiers]),
             _ => true,
             "commandA",
           );

      // Remap b -> <Plug>a
      let (bindings, _id) =
        bindings
        |> Input.addMapping(
             ~allowRecursive=true,
             Sequence([bKeyNoModifiers]),
             _ => true,
             [plugKey, aKeyNoModifiers],
           );

      // Pressing b should remap to <Plug>a, which should execute "commandA"
      let (_bindings, effects) =
        Input.keyDown(
          ~context=true,
          ~scancode=bKeyScancode,
          ~key=candidate(bKeyNoModifiers),
          bindings,
        );
      expect.equal(effects, [Execute("commandA")]);
    });

    test("leader key can participate in remap", ({expect, _}) => {
      // Add a <Leader>a -> "commandLeaderA" mapping
      let (bindings, _id) =
        Input.empty
        |> Input.addBinding(
             Sequence([leaderKey, aKeyNoModifiers]),
             _ => true,
             "commandLeaderA",
           );

      // Remap b -> <Leadeer>
      let (bindings, _id) =
        bindings
        |> Input.addMapping(
             ~allowRecursive=true,
             Sequence([bKeyNoModifiers]),
             _ => true,
             [leaderKey],
           );

      // Pressing b, as the leader key...
      let (bindings, effects) =
        Input.keyDown(
          ~context=true,
          ~scancode=bKeyScancode,
          ~key=candidate(bKeyNoModifiers),
          bindings,
        );
      expect.equal(effects, []);

      // And then a to complete the binding
      let (_bindings, effects) =
        Input.keyDown(
          ~context=true,
          ~scancode=aKeyScancode,
          ~key=candidate(aKeyNoModifiers),
          bindings,
        );
      expect.equal(effects, [Execute("commandLeaderA")]);
    });

    test("leader key defined as a", ({expect, _}) => {
      let physicalKey =
        PhysicalKey.{
          key: Key.Character(Uchar.of_char('a')),
          modifiers: Modifiers.none,
        };
      let (bindings, _id) =
        Input.empty
        |> Input.addBinding(Sequence([leaderKey]), _ => true, "commandA");

      let (_bindings, effects) =
        Input.keyDown(
          ~leaderKey=Some(physicalKey),
          ~context=true,
          ~scancode=aKeyScancode,
          ~key=candidate(aKeyNoModifiers),
          bindings,
        );
      expect.equal(effects, [Execute("commandA")]);
    });
  });
  describe("leader key", ({test, _}) => {
    test("no leader key defined", ({expect, _}) => {
      let (bindings, _id) =
        Input.empty
        |> Input.addBinding(Sequence([leaderKey]), _ => true, "commandA");

      let (_bindings, effects) =
        Input.keyDown(
          ~context=true,
          ~scancode=aKeyScancode,
          ~key=candidate(aKeyNoModifiers),
          bindings,
        );
      expect.equal(
        effects,
        [
          Unhandled({
            key: candidate(aKeyNoModifiers),
            isProducedByRemap: false,
          }),
        ],
      );
    });

    test("leader key defined as a", ({expect, _}) => {
      let physicalKey =
        PhysicalKey.{
          key: Key.Character(Uchar.of_char('a')),
          modifiers: Modifiers.none,
        };
      let (bindings, _id) =
        Input.empty
        |> Input.addBinding(Sequence([leaderKey]), _ => true, "commandA");

      let (_bindings, effects) =
        Input.keyDown(
          ~leaderKey=Some(physicalKey),
          ~scancode=aKeyScancode,
          ~context=true,
          ~key=candidate(aKeyNoModifiers),
          bindings,
        );
      expect.equal(effects, [Execute("commandA")]);
    });
  });
  describe("allKeysReleased", ({test, _}) => {
    test("basic release case", ({expect, _}) => {
      let (bindings, _id) =
        Input.empty
        |> Input.addBinding(AllKeysReleased, _ => true, "commandA");

      let (bindings, effects) =
        Input.keyDown(
          ~context=true,
          ~scancode=aKeyScancode,
          ~key=candidate(aKeyNoModifiers),
          bindings,
        );
      expect.equal(
        effects,
        [
          Unhandled({
            key: candidate(aKeyNoModifiers),
            isProducedByRemap: false,
          }),
        ],
      );

      let (_bindings, effects) =
        Input.keyUp(~context=true, ~scancode=aKeyScancode, bindings);

      expect.equal(effects, [Execute("commandA")]);
    })
  });
  describe("text", ({test, _}) => {
    test("should immediately dispatch if no pending keys", ({expect, _}) => {
      let (bindings, _id) =
        Input.empty
        |> Input.addBinding(
             Sequence([aKeyNoModifiers]),
             _ => true,
             "commandA",
           );

      let (_bindings, effects) = Input.text(~text="a", bindings);
      expect.equal(effects, [Text("a")]);
    });
    test("should be held in sequence", ({expect, _}) => {
      let (bindings, _id) =
        Input.empty
        |> Input.addBinding(
             Sequence([aKeyNoModifiers, bKeyNoModifiers]),
             _ => true,
             "commandAB",
           );

      let (bindings, _effects) =
        Input.keyDown(
          ~context=true,
          ~scancode=aKeyScancode,
          ~key=candidate(aKeyNoModifiers),
          bindings,
        );

      let (bindings, _effects) = Input.text(~text="a", bindings);

      // Sequence fails - get text event
      let (_bindings, effects) =
        Input.keyDown(
          ~context=true,
          ~scancode=aKeyScancode,
          ~key=candidate(cKeyNoModifiers),
          bindings,
        );

      expect.equal(
        effects,
        [
          Text("a"),
          Unhandled({
            key: candidate(cKeyNoModifiers),
            isProducedByRemap: false,
          }),
        ],
      );
    });

    test(
      "text after successful binding should not be dispatched", ({expect, _}) => {
      let (bindings, _id) =
        Input.empty
        |> Input.addBinding(
             Sequence([aKeyNoModifiers, bKeyNoModifiers]),
             _ => true,
             "commandAB",
           );

      let (bindings, _effects) =
        Input.keyDown(
          ~context=true,
          ~scancode=aKeyScancode,
          ~key=candidate(aKeyNoModifiers),
          bindings,
        );

      let (bindings, _effects) = Input.text(~text="a", bindings);

      // Sequence fails - get text event
      let (bindings, effects) =
        Input.keyDown(
          ~context=true,
          ~scancode=bKeyScancode,
          ~key=candidate(bKeyNoModifiers),
          bindings,
        );

      expect.equal(effects, [Execute("commandAB")]);

      let (_bindings, effects) = Input.text(~text="b", bindings);

      // Subsequent text input should be ignored
      expect.equal(effects, []);
    });

    test(
      "#3048: text with new keydown after successful binding should be dispatched",
      ({expect, _}) => {
      let (bindings, _id) =
        Input.empty
        |> Input.addBinding(
             Sequence([aKeyNoModifiers]),
             _ => true,
             "commandA",
           );

      let (bindings, effects) =
        Input.keyDown(
          ~context=true,
          ~scancode=aKeyScancode,
          ~key=candidate(aKeyNoModifiers),
          bindings,
        );

      // Keydown should trigger command...
      expect.equal(effects, [Execute("commandA")]);

      let (bindings, _effects) = Input.text(~text="a", bindings);

      let (bindings, _effects) =
        Input.keyDown(
          ~context=true,
          ~scancode=bKeyScancode,
          ~key=candidate(bKeyNoModifiers),
          bindings,
        );

      let (_bindings, effects) = Input.text(~text="b", bindings);

      // Subsequent text input should be triggered, even though we haven't had a KeyUp for "a" yet
      expect.equal(effects, [Text("b")]);
    });

    test("text not dispatched in sequence", ({expect, _}) => {
      let (bindings, _id) =
        Input.empty
        |> Input.addBinding(
             Sequence([aKeyNoModifiers, bKeyNoModifiers]),
             _ => true,
             "commandAB",
           );

      let (bindings, _effects) =
        Input.keyDown(
          ~context=true,
          ~scancode=aKeyScancode,
          ~key=candidate(aKeyNoModifiers),
          bindings,
        );

      let (bindings, _effects) = Input.text(~text="a", bindings);

      // Sequence fails - get text event
      let (_bindings, effects) =
        Input.keyDown(
          ~context=true,
          ~scancode=bKeyScancode,
          ~key=candidate(bKeyNoModifiers),
          bindings,
        );

      expect.equal(effects, [Execute("commandAB")]);
    });
  });
  describe("sequences", ({test, _}) => {
    test("simple sequence", ({expect, _}) => {
      let (bindings, _id) =
        Input.empty
        |> Input.addBinding(
             Sequence([aKeyNoModifiers, bKeyNoModifiers]),
             _ => true,
             "command1",
           );

      expect.equal(Input.isPending(bindings), false);

      let (bindings, effects) =
        Input.keyDown(
          ~context=true,
          ~scancode=aKeyScancode,
          ~key=candidate(aKeyNoModifiers),
          bindings,
        );

      expect.equal(Input.isPending(bindings), true);
      expect.equal(effects, []);

      let (bindings, effects) =
        Input.keyDown(
          ~context=true,
          ~scancode=bKeyScancode,
          ~key=candidate(bKeyNoModifiers),
          bindings,
        );

      expect.equal(effects, [Execute("command1")]);
      expect.equal(Input.isPending(bindings), false);
    });
    test("same key in a sequence", ({expect, _}) => {
      let (bindings, _id) =
        Input.empty
        |> Input.addBinding(
             Sequence([aKeyNoModifiers, aKeyNoModifiers]),
             _ => true,
             "commandAA",
           );

      let (bindings, effects) =
        Input.keyDown(
          ~context=true,
          ~scancode=aKeyScancode,
          ~key=candidate(aKeyNoModifiers),
          bindings,
        );

      expect.equal(effects, []);

      let (_bindings, effects) =
        Input.keyDown(
          ~context=true,
          ~scancode=aKeyScancode,
          ~key=candidate(aKeyNoModifiers),
          bindings,
        );

      expect.equal(effects, [Execute("commandAA")]);
    });
    test("key up doesn't stop sequence", ({expect, _}) => {
      let (bindings, _id) =
        Input.empty
        |> Input.addBinding(
             Sequence([aKeyNoModifiers, bKeyNoModifiers]),
             _ => true,
             "commandAB",
           );

      let (bindings, effects) =
        Input.keyDown(
          ~context=true,
          ~scancode=aKeyScancode,
          ~key=candidate(aKeyNoModifiers),
          bindings,
        );

      expect.equal(effects, []);

      let (bindings, effects) =
        Input.keyUp(~context=true, ~scancode=aKeyScancode, bindings);

      expect.equal(effects, []);

      let (_bindings, effects) =
        Input.keyDown(
          ~context=true,
          ~scancode=aKeyScancode,
          ~key=candidate(bKeyNoModifiers),
          bindings,
        );

      expect.equal(effects, [Execute("commandAB")]);
    });
    //    test("sequence with keyups", ({expect, _}) => {
    //      let (bindings, _id) =
    //        Input.empty
    //        |> Input.addBinding(
    //             Sequence([
    //               Keydown(Keycode(1, Modifiers.none)),
    //               Keyup(Keycode(1, Modifiers.none)),
    //             ]),
    //             _ => true,
    //             "commandA!A",
    //           );
    //
    //      let (bindings, effects) =
    //        Input.keyDown(~context=true, ~key=aKeyNoModifiers, bindings);
    //
    //      expect.equal(effects, []);
    //
    //      let (_bindings, effects) =
    //        Input.keyUp(~context=true, ~key=aKeyNoModifiers, bindings);
    //
    //      expect.equal(effects, [Execute("commandA!A")]);
    //    });
    test("partial match with unhandled", ({expect, _}) => {
      let (bindings, _id) =
        Input.empty
        |> Input.addBinding(
             Sequence([aKeyNoModifiers, cKeyNoModifiers]),
             _ => true,
             "commandAC",
           );

      let (bindings, _id) =
        bindings
        |> Input.addBinding(
             Sequence([aKeyNoModifiers]),
             _ => true,
             "commandA",
           );

      let (bindings, effects) =
        Input.keyDown(
          ~context=true,
          ~scancode=aKeyScancode,
          ~key=candidate(aKeyNoModifiers),
          bindings,
        );

      expect.equal(effects, []);

      let (_bindings, effects) =
        Input.keyDown(
          ~context=true,
          ~scancode=bKeyScancode,
          ~key=candidate(bKeyNoModifiers),
          bindings,
        );

      expect.equal(
        effects,
        [
          Execute("commandA"),
          Unhandled({
            key: candidate(bKeyNoModifiers),
            isProducedByRemap: false,
          }),
        ],
      );
    });
    test("#1691: almost match gets unhandled", ({expect, _}) => {
      // Add binding for 'aa'
      let (bindings, _id) =
        Input.empty
        |> Input.addBinding(
             Sequence([aKeyNoModifiers, aKeyNoModifiers]),
             _ => true,
             "commandAC",
           );

      // Press a...
      let (bindings, effects) =
        Input.keyDown(
          ~context=true,
          ~scancode=aKeyScancode,
          ~key=candidate(aKeyNoModifiers),
          bindings,
        );

      expect.equal(effects, []);

      // Press b...
      let (_bindings, effects) =
        Input.keyDown(
          ~context=true,
          ~scancode=bKeyScancode,
          ~key=candidate(bKeyNoModifiers),
          bindings,
        );

      expect.equal(
        effects,
        [
          Unhandled({
            key: candidate(aKeyNoModifiers),
            isProducedByRemap: false,
          }),
          Unhandled({
            key: candidate(bKeyNoModifiers),
            isProducedByRemap: false,
          }),
        ],
      );
    });
  });
  describe("enabled / disabled", ({test, _}) => {
    let identity = context => context;
    test("unhandled when enabled function returns false", ({expect, _}) => {
      let (bindings, _id) =
        Input.empty
        |> Input.addBinding(
             Sequence([aKeyNoModifiers]),
             identity,
             "command1",
           );

      let (_bindings, effects) =
        Input.keyDown(
          ~context=false,
          ~scancode=aKeyScancode,
          ~key=candidate(aKeyNoModifiers),
          bindings,
        );

      // Should be unhandled because the context function is [false]
      expect.equal(
        effects,
        [
          Unhandled({
            key: candidate(aKeyNoModifiers),
            isProducedByRemap: false,
          }),
        ],
      );
    });
    test("key sequence is unhandled when context is false", ({expect, _}) => {
      let (bindings, _id) =
        Input.empty
        |> Input.addBinding(
             Sequence([aKeyNoModifiers, cKeyNoModifiers]),
             identity,
             "commandAC",
           );

      let (_bindings, effects) =
        Input.keyDown(
          ~context=false,
          ~scancode=aKeyScancode,
          ~key=candidate(aKeyNoModifiers),
          bindings,
        );

      expect.equal(
        effects,
        [
          Unhandled({
            key: candidate(aKeyNoModifiers),
            isProducedByRemap: false,
          }),
        ],
      );
    });
  });
  describe("key matching", ({test, _}) => {
    test("matches keycode", ({expect, _}) => {
      let (bindings, _id) =
        Input.empty
        |> Input.addBinding(
             Sequence([aKeyNoModifiers]),
             _ => true,
             "command1",
           );

      let (_bindings, effects) =
        Input.keyDown(
          ~context=true,
          ~scancode=aKeyScancode,
          ~key=candidate(aKeyNoModifiers),
          bindings,
        );

      expect.equal(effects, [Execute("command1")]);
    });
    test("key with no matches is unhandled", ({expect, _}) => {
      let bindings = Input.empty;

      let (_bindings, effects) =
        Input.keyDown(
          ~context=true,
          ~scancode=aKeyScancode,
          ~key=candidate(aKeyNoModifiers),
          bindings,
        );

      expect.equal(
        effects,
        [
          Unhandled({
            key: candidate(aKeyNoModifiers),
            isProducedByRemap: false,
          }),
        ],
      );
    });
  });
  describe("utf8", ({test, _}) => {
    let uc252 = Zed_utf8.get("ü", 0);
    let sc252 = 252;

    let uc246 = Zed_utf8.get("ö", 0);

    test("map utf8 -> ascii", ({expect, _}) => {
      let (bindings, _id) =
        Input.empty
        |> Input.addMapping(
             ~allowRecursive=false,
             Sequence([ucharNoModifiers(uc252)]),
             _ => true,
             [aKeyNoModifiers],
           );

      let (_bindings, effects) =
        Input.keyDown(
          ~context=true,
          ~scancode=sc252,
          ~key=candidate(ucharNoModifiers(uc252)),
          bindings,
        );

      expect.equal(
        effects,
        [
          Unhandled({
            key: candidate(aKeyNoModifiers),
            isProducedByRemap: true,
          }),
        ],
      );
    });

    test("map ascii -> utf8", ({expect, _}) => {
      let (bindings, _id) =
        Input.empty
        |> Input.addMapping(
             ~allowRecursive=false,
             Sequence([aKeyNoModifiers]),
             _ => true,
             [ucharNoModifiers(uc246)],
           );

      let (_bindings, effects) =
        Input.keyDown(
          ~context=true,
          ~scancode=aKeyScancode,
          ~key=candidate(aKeyNoModifiers),
          bindings,
        );

      expect.equal(
        effects,
        [
          Unhandled({
            key: candidate(ucharNoModifiers(uc246)),
            isProducedByRemap: true,
          }),
        ],
      );
    });
  });
  describe("remapping", ({test, _}) => {
    test("no-recursive mapping", ({expect, _}) => {
      let (bindings, _id) =
        Input.empty
        |> Input.addMapping(
             ~allowRecursive=false,
             Sequence([aKeyNoModifiers]),
             _ => true,
             [bKeyNoModifiers],
           );

      let (bindings, _id) =
        bindings
        |> Input.addMapping(
             ~allowRecursive=false,
             Sequence([bKeyNoModifiers]),
             _ => true,
             [cKeyNoModifiers],
           );
      let (_bindings, effects) =
        Input.keyDown(
          ~context=true,
          ~scancode=aKeyScancode,
          ~key=candidate(aKeyNoModifiers),
          bindings,
        );

      expect.equal(
        effects,
        [
          Unhandled({
            key: candidate(bKeyNoModifiers),
            isProducedByRemap: true,
          }),
        ],
      );
    });

    test("#3729: no-recursive mapping emits in correct order", ({expect, _}) => {
      let (bindings, _id) =
        Input.empty
        |> Input.addBinding(
             Sequence([aKeyNoModifiers, bKeyNoModifiers]),
             _ => true,
             "testCommand",
           );

      let (bindings, _id) =
        bindings
        |> Input.addMapping(
             ~allowRecursive=false,
             Sequence([cKeyNoModifiers]),
             _ => true,
             // Important for repro - the initial binding (aKeyNoModifiers) must be used by an existing binding.
             [aKeyNoModifiers, cKeyNoModifiers],
           );

      let (_bindings, effects) =
        Input.keyDown(
          ~context=true,
          ~scancode=cKeyScancode,
          ~key=candidate(cKeyNoModifiers),
          bindings,
        );

      expect.equal(
        effects,
        [
          Unhandled({
            key: candidate(aKeyNoModifiers),
            isProducedByRemap: true,
          }),
          Unhandled({
            key: candidate(cKeyNoModifiers),
            isProducedByRemap: true,
          }),
        ],
      );
    });

    test("unhandled, single key remap", ({expect, _}) => {
      let (bindings, _id) =
        Input.empty
        |> Input.addMapping(
             ~allowRecursive=true,
             Sequence([aKeyNoModifiers]),
             _ => true,
             [bKeyNoModifiers],
           );

      let (_bindings, effects) =
        Input.keyDown(
          ~context=true,
          ~scancode=aKeyScancode,
          ~key=candidate(aKeyNoModifiers),
          bindings,
        );

      expect.equal(
        effects,
        [
          Unhandled({
            key: candidate(bKeyNoModifiers),
            isProducedByRemap: true,
          }),
        ],
      );
    });

    test("2-step recursive mapping", ({expect, _}) => {
      let (bindings, _id) =
        Input.empty
        |> Input.addMapping(
             ~allowRecursive=true,
             Sequence([aKeyNoModifiers]),
             _ => true,
             [bKeyNoModifiers],
           );

      let (bindings, _id) =
        bindings
        |> Input.addMapping(
             ~allowRecursive=true,
             Sequence([bKeyNoModifiers]),
             _ => true,
             [cKeyNoModifiers],
           );

      let (_bindings, effects) =
        Input.keyDown(
          ~context=true,
          ~scancode=aKeyScancode,
          ~key=candidate(aKeyNoModifiers),
          bindings,
        );

      expect.equal(
        effects,
        [
          Unhandled({
            key: candidate(cKeyNoModifiers),
            isProducedByRemap: true,
          }),
        ],
      );
    });

    test("recursive mapping doesn't hang", ({expect, _}) => {
      let (bindings, _id) =
        Input.empty
        |> Input.addMapping(
             ~allowRecursive=true,
             Sequence([aKeyNoModifiers]),
             _ => true,
             [aKeyNoModifiers],
           );

      let (_bindings, effects) =
        Input.keyDown(
          ~context=true,
          ~scancode=aKeyScancode,
          ~key=candidate(aKeyNoModifiers),
          bindings,
        );

      expect.equal(
        effects,
        [
          Unhandled({
            key: candidate(aKeyNoModifiers),
            isProducedByRemap: true,
          }),
          RemapRecursionLimitHit,
        ],
      );
    });

    test("unhandled, sequence remap", ({expect, _}) => {
      let (bindings, _id) =
        Input.empty
        |> Input.addMapping(
             ~allowRecursive=true,
             Sequence([aKeyNoModifiers, bKeyNoModifiers]),
             _ => true,
             [cKeyNoModifiers],
           );

      let (bindings, effects) =
        Input.keyDown(
          ~context=true,
          ~scancode=aKeyScancode,
          ~key=candidate(aKeyNoModifiers),
          bindings,
        );
      expect.equal(effects, []);
      let (_bindings, effects) =
        Input.keyDown(
          ~context=true,
          ~scancode=bKeyScancode,
          ~key=candidate(bKeyNoModifiers),
          bindings,
        );

      expect.equal(
        effects,
        [
          Unhandled({
            key: candidate(cKeyNoModifiers),
            isProducedByRemap: true,
          }),
        ],
      );
    });
    test("unhandled, multiple keys", ({expect, _}) => {
      let (bindings, _id) =
        Input.empty
        |> Input.addMapping(
             ~allowRecursive=false,
             Sequence([aKeyNoModifiers]),
             _ => true,
             [bKeyNoModifiers, cKeyNoModifiers],
           );

      let (_bindings, effects) =
        Input.keyDown(
          ~context=true,
          ~scancode=aKeyScancode,
          ~key=candidate(aKeyNoModifiers),
          bindings,
        );

      expect.equal(
        effects,
        [
          Unhandled({
            key: candidate(bKeyNoModifiers),
            isProducedByRemap: true,
          }),
          Unhandled({
            key: candidate(cKeyNoModifiers),
            isProducedByRemap: true,
          }),
        ],
      );
    });
    test("with command", ({expect, _}) => {
      let (bindings, _id) =
        Input.empty
        |> Input.addMapping(
             ~allowRecursive=true,
             Sequence([aKeyNoModifiers]),
             _ => true,
             [bKeyNoModifiers],
           );
      let (bindings, _id) =
        bindings
        |> Input.addBinding(
             Sequence([aKeyNoModifiers]),
             _ => true,
             "command2",
           );

      let (_bindings, effects) =
        Input.keyDown(
          ~context=true,
          ~scancode=aKeyScancode,
          ~key=candidate(aKeyNoModifiers),
          bindings,
        );

      expect.equal(effects, [Execute("command2")]);
    });
    test("with multiple commands", ({expect, _}) => {
      let (bindings, _id) =
        Input.empty
        |> Input.addMapping(
             ~allowRecursive=true,
             Sequence([aKeyNoModifiers]),
             _ => true,
             [bKeyNoModifiers, cKeyNoModifiers],
           );
      let (bindings, _id) =
        bindings
        |> Input.addBinding(
             Sequence([bKeyNoModifiers]),
             _ => true,
             "command2",
           );

      let (bindings, _id) =
        bindings
        |> Input.addBinding(
             Sequence([cKeyNoModifiers]),
             _ => true,
             "command3",
           );

      let (_bindings, effects) =
        Input.keyDown(
          ~context=true,
          ~scancode=aKeyScancode,
          ~key=candidate(aKeyNoModifiers),
          bindings,
        );

      expect.equal(effects, [Execute("command2"), Execute("command3")]);
    });
  });

  describe("enable / disable", ({test, _}) => {
    test("unhandled when bindings are disabled", ({expect, _}) => {
      let alwaysTrue = _ => true;
      let (bindings, _id) =
        Input.empty
        |> Input.addBinding(
             Sequence([aKeyNoModifiers]),
             alwaysTrue,
             "command1",
           );

      let bindings' = Input.disable(bindings);

      let (_bindings, effects) =
        Input.keyDown(
          ~context=true,
          ~scancode=aKeyScancode,
          ~key=candidate(aKeyNoModifiers),
          bindings',
        );

      // Should be unhandled because the context function is [false]
      expect.equal(
        effects,
        [
          Unhandled({
            key: candidate(aKeyNoModifiers),
            isProducedByRemap: false,
          }),
        ],
      );
    })
  });

  describe("timeout", ({test, _}) => {
    test("timeout with partial match", ({expect, _}) => {
      let (bindings, _id) =
        Input.empty
        |> Input.addBinding(
             Sequence([aKeyNoModifiers, cKeyNoModifiers]),
             _ => true,
             "commandAC",
           );

      let (bindings, _id) =
        bindings
        |> Input.addBinding(
             Sequence([aKeyNoModifiers]),
             _ => true,
             "commandA",
           );

      let (bindings, effects) =
        Input.keyDown(
          ~context=true,
          ~scancode=aKeyScancode,
          ~key=candidate(aKeyNoModifiers),
          bindings,
        );

      expect.equal(effects, []);

      let (_bindings, effects) = Input.timeout(~context=true, bindings);

      expect.equal(effects, [Execute("commandA")]);
    });
    test("timeout with no match", ({expect, _}) => {
      let (bindings, _id) =
        Input.empty
        |> Input.addBinding(
             Sequence([aKeyNoModifiers, cKeyNoModifiers]),
             _ => true,
             "commandAC",
           );

      let (bindings, effects) =
        Input.keyDown(
          ~context=true,
          ~scancode=aKeyScancode,
          ~key=candidate(aKeyNoModifiers),
          bindings,
        );

      expect.equal(effects, []);

      let (_bindings, effects) = Input.timeout(~context=true, bindings);

      expect.equal(
        effects,
        [
          Unhandled({
            key: candidate(aKeyNoModifiers),
            isProducedByRemap: false,
          }),
        ],
      );
    });
    test("timeout with no match, but prior text event", ({expect, _}) => {
      let (bindings, _id) =
        Input.empty
        |> Input.addBinding(
             Sequence([aKeyNoModifiers, cKeyNoModifiers]),
             _ => true,
             "commandAC",
           );

      let (bindings, effects) =
        Input.keyDown(
          ~context=true,
          ~scancode=aKeyScancode,
          ~key=candidate(aKeyNoModifiers),
          bindings,
        );

      expect.equal(effects, []);

      let (bindings, effects) = Input.text(~text="a", bindings);

      expect.equal(effects, []);

      let (_bindings, effects) = Input.timeout(~context=true, bindings);

      expect.equal(effects, [Text("a")]);
    });
  });
});
