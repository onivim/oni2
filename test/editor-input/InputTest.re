open TestFramework;
open EditorInput;

let aKeyNoModifiers =
  KeyPress.physicalKey(~scancode=101, ~keycode=1, ~modifiers=Modifiers.none);

let bKeyNoModifiers =
  KeyPress.physicalKey(~scancode=102, ~keycode=2, ~modifiers=Modifiers.none);

let cKeyNoModifiers =
  KeyPress.physicalKey(~scancode=103, ~keycode=3, ~modifiers=Modifiers.none);

let leaderKey = KeyPress.specialKey(SpecialKey.Leader);
let plugKey = KeyPress.specialKey(SpecialKey.Plug);

module Input =
  EditorInput.Make({
    type command = string;
    type context = bool;
  });

describe("EditorInput", ({describe, _}) => {
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
             Sequence([bKeyNoModifiers]),
             _ => true,
             [plugKey, aKeyNoModifiers],
           );

      // Pressing b should remap to <Plug>a, which should execute "commandA"
      let (_bindings, effects) =
        Input.keyDown(~context=true, ~key=bKeyNoModifiers, bindings);
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
             Sequence([bKeyNoModifiers]),
             _ => true,
             [leaderKey],
           );

      // Pressing b, as the leader key...
      let (bindings, effects) =
        Input.keyDown(~context=true, ~key=bKeyNoModifiers, bindings);
      expect.equal(effects, []);

      // And then a to complete the binding
      let (_bindings, effects) =
        Input.keyDown(~context=true, ~key=aKeyNoModifiers, bindings);
      expect.equal(effects, [Execute("commandLeaderA")]);
    });

    test("leader key defined as a", ({expect, _}) => {
      let physicalKey =
        PhysicalKey.{scancode: 101, keycode: 1, modifiers: Modifiers.none};
      let (bindings, _id) =
        Input.empty
        |> Input.addBinding(Sequence([leaderKey]), _ => true, "commandA");

      let (_bindings, effects) =
        Input.keyDown(
          ~leaderKey=Some(physicalKey),
          ~context=true,
          ~key=aKeyNoModifiers,
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
        Input.keyDown(~context=true, ~key=aKeyNoModifiers, bindings);
      expect.equal(effects, [Unhandled(aKeyNoModifiers)]);
    });

    test("leader key defined as a", ({expect, _}) => {
      let physicalKey =
        PhysicalKey.{scancode: 101, keycode: 1, modifiers: Modifiers.none};
      let (bindings, _id) =
        Input.empty
        |> Input.addBinding(Sequence([leaderKey]), _ => true, "commandA");

      let (_bindings, effects) =
        Input.keyDown(
          ~leaderKey=Some(physicalKey),
          ~context=true,
          ~key=aKeyNoModifiers,
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
        Input.keyDown(~context=true, ~key=aKeyNoModifiers, bindings);
      expect.equal(effects, [Unhandled(aKeyNoModifiers)]);

      let (_bindings, effects) =
        Input.keyUp(~context=true, ~key=aKeyNoModifiers, bindings);

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
        Input.keyDown(~context=true, ~key=aKeyNoModifiers, bindings);

      let (bindings, _effects) = Input.text(~text="a", bindings);

      // Sequence fails - get text event
      let (_bindings, effects) =
        Input.keyDown(~context=true, ~key=cKeyNoModifiers, bindings);

      expect.equal(effects, [Text("a"), Unhandled(cKeyNoModifiers)]);
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
        Input.keyDown(~context=true, ~key=aKeyNoModifiers, bindings);

      let (bindings, _effects) = Input.text(~text="a", bindings);

      // Sequence fails - get text event
      let (bindings, effects) =
        Input.keyDown(~context=true, ~key=bKeyNoModifiers, bindings);

      expect.equal(effects, [Execute("commandAB")]);

      let (_bindings, effects) = Input.text(~text="b", bindings);

      // Subsequent text input should be ignored
      expect.equal(effects, []);
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
        Input.keyDown(~context=true, ~key=aKeyNoModifiers, bindings);

      let (bindings, _effects) = Input.text(~text="a", bindings);

      // Sequence fails - get text event
      let (_bindings, effects) =
        Input.keyDown(~context=true, ~key=bKeyNoModifiers, bindings);

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
        Input.keyDown(~context=true, ~key=aKeyNoModifiers, bindings);

      expect.equal(Input.isPending(bindings), true);
      expect.equal(effects, []);

      let (bindings, effects) =
        Input.keyDown(~context=true, ~key=bKeyNoModifiers, bindings);

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
        Input.keyDown(~context=true, ~key=aKeyNoModifiers, bindings);

      expect.equal(effects, []);

      let (_bindings, effects) =
        Input.keyDown(~context=true, ~key=aKeyNoModifiers, bindings);

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
        Input.keyDown(~context=true, ~key=aKeyNoModifiers, bindings);

      expect.equal(effects, []);

      let (bindings, effects) =
        Input.keyUp(~context=true, ~key=aKeyNoModifiers, bindings);

      expect.equal(effects, []);

      let (_bindings, effects) =
        Input.keyDown(~context=true, ~key=bKeyNoModifiers, bindings);

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
        Input.keyDown(~context=true, ~key=aKeyNoModifiers, bindings);

      expect.equal(effects, []);

      let (_bindings, effects) =
        Input.keyDown(~context=true, ~key=bKeyNoModifiers, bindings);

      expect.equal(
        effects,
        [Execute("commandA"), Unhandled(bKeyNoModifiers)],
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
        Input.keyDown(~context=true, ~key=aKeyNoModifiers, bindings);

      expect.equal(effects, []);

      // Press b...
      let (_bindings, effects) =
        Input.keyDown(~context=true, ~key=bKeyNoModifiers, bindings);

      expect.equal(
        effects,
        [Unhandled(aKeyNoModifiers), Unhandled(bKeyNoModifiers)],
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
        Input.keyDown(~context=false, ~key=aKeyNoModifiers, bindings);

      // Should be unhandled because the context function is [false]
      expect.equal(effects, [Unhandled(aKeyNoModifiers)]);
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
        Input.keyDown(~context=false, ~key=aKeyNoModifiers, bindings);

      expect.equal(effects, [Unhandled(aKeyNoModifiers)]);
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
        Input.keyDown(~context=true, ~key=aKeyNoModifiers, bindings);

      expect.equal(effects, [Execute("command1")]);
    });
    test("key with no matches is unhandled", ({expect, _}) => {
      let bindings = Input.empty;

      let (_bindings, effects) =
        Input.keyDown(~context=true, ~key=aKeyNoModifiers, bindings);

      expect.equal(effects, [Unhandled(aKeyNoModifiers)]);
    });
  });
  describe("remapping", ({test, _}) => {
    test("unhandled, single key remap", ({expect, _}) => {
      let (bindings, _id) =
        Input.empty
        |> Input.addMapping(
             Sequence([aKeyNoModifiers]),
             _ => true,
             [bKeyNoModifiers],
           );

      let (_bindings, effects) =
        Input.keyDown(~context=true, ~key=aKeyNoModifiers, bindings);

      expect.equal(effects, [Unhandled(bKeyNoModifiers)]);
    });

    test("2-step recursive mapping", ({expect, _}) => {
      let (bindings, _id) =
        Input.empty
        |> Input.addMapping(
             Sequence([aKeyNoModifiers]),
             _ => true,
             [bKeyNoModifiers],
           );

      let (bindings, _id) =
        bindings
        |> Input.addMapping(
             Sequence([bKeyNoModifiers]),
             _ => true,
             [cKeyNoModifiers],
           );

      let (_bindings, effects) =
        Input.keyDown(~context=true, ~key=aKeyNoModifiers, bindings);

      expect.equal(effects, [Unhandled(cKeyNoModifiers)]);
    });

    test("recursive mapping doesn't hang", ({expect, _}) => {
      let (bindings, _id) =
        Input.empty
        |> Input.addMapping(
             Sequence([aKeyNoModifiers]),
             _ => true,
             [aKeyNoModifiers],
           );

      let (_bindings, effects) =
        Input.keyDown(~context=true, ~key=aKeyNoModifiers, bindings);

      expect.equal(
        effects,
        [RemapRecursionLimitHit, Unhandled(aKeyNoModifiers)],
      );
    });

    test("unhandled, sequence remap", ({expect, _}) => {
      let (bindings, _id) =
        Input.empty
        |> Input.addMapping(
             Sequence([aKeyNoModifiers, bKeyNoModifiers]),
             _ => true,
             [cKeyNoModifiers],
           );

      let (bindings, effects) =
        Input.keyDown(~context=true, ~key=aKeyNoModifiers, bindings);
      expect.equal(effects, []);
      let (_bindings, effects) =
        Input.keyDown(~context=true, ~key=bKeyNoModifiers, bindings);

      expect.equal(effects, [Unhandled(cKeyNoModifiers)]);
    });
    test("unhandled, multiple keys", ({expect, _}) => {
      let (bindings, _id) =
        Input.empty
        |> Input.addMapping(
             Sequence([aKeyNoModifiers]),
             _ => true,
             [bKeyNoModifiers, cKeyNoModifiers],
           );

      let (_bindings, effects) =
        Input.keyDown(~context=true, ~key=aKeyNoModifiers, bindings);

      expect.equal(
        effects,
        [Unhandled(bKeyNoModifiers), Unhandled(cKeyNoModifiers)],
      );
    });
    test("with command", ({expect, _}) => {
      let (bindings, _id) =
        Input.empty
        |> Input.addMapping(
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
        Input.keyDown(~context=true, ~key=aKeyNoModifiers, bindings);

      expect.equal(effects, [Execute("command2")]);
    });
    test("with multiple commands", ({expect, _}) => {
      let (bindings, _id) =
        Input.empty
        |> Input.addMapping(
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
        Input.keyDown(~context=true, ~key=aKeyNoModifiers, bindings);

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
        Input.keyDown(~context=true, ~key=aKeyNoModifiers, bindings');

      // Should be unhandled because the context function is [false]
      expect.equal(effects, [Unhandled(aKeyNoModifiers)]);
    })
  });
});
