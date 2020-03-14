open TestFramework;
open Oni_Core;
open Oni_Core.Input2;

let aKeyNoModifiers: key = {
  scancode: 101,
  keycode: 1,
  modifiers: Modifiers.none,
  text: "a",
};

let bKeyNoModifiers: key = {
  scancode: 102,
  keycode: 2,
  modifiers: Modifiers.none,
  text: "b",
};

let cKeyNoModifiers: key = {
  scancode: 103,
  keycode: 3,
  modifiers: Modifiers.none,
  text: "c",
};

describe("Input 2", ({describe, _}) => {
  describe("flush", ({test, _}) => {
    test("simple sequence", ({expect}) => {
      let (bindings, _id) =
        Input2.empty
        |> Input2.addBinding(
             [Keycode(1, Modifiers.none), Keycode(2, Modifiers.none)],
             _ => true,
             "payloadAB",
           );
      
      let (bindings, _id) =
        bindings
        |> Input2.addBinding(
             [Keycode(1, Modifiers.none)],
             _ => true,
             "payloadA",
           );

      let (bindings, effects) = Input2.keyDown(aKeyNoModifiers, bindings);

      expect.equal(effects, []);

      let (bindings, effects) = Input2.flush(bindings);

      expect.equal(effects, [Execute("payloadA")]);
    });
  });
  describe("sequences", ({test, _}) => {
    test("simple sequence", ({expect}) => {
      let (bindings, _id) =
        Input2.empty
        |> Input2.addBinding(
             [Keycode(1, Modifiers.none), Keycode(2, Modifiers.none)],
             _ => true,
             "payload1",
           );

      let (bindings, effects) = Input2.keyDown(aKeyNoModifiers, bindings);

      expect.equal(effects, []);

      let (bindings, effects) = Input2.keyDown(bKeyNoModifiers, bindings);

      expect.equal(effects, [Execute("payload1")]);
    });
    test("same key in a sequence", ({expect}) => {
      let (bindings, _id) =
        Input2.empty
        |> Input2.addBinding(
             [Keycode(1, Modifiers.none), Keycode(1, Modifiers.none)],
             _ => true,
             "payloadAA",
           );

      let (bindings, effects) = Input2.keyDown(aKeyNoModifiers, bindings);

      expect.equal(effects, []);

      let (bindings, effects) = Input2.keyDown(aKeyNoModifiers, bindings);

      expect.equal(effects, [Execute("payloadAA")]);
    });
    test("partial match with another match", ({expect}) => {
      let (bindings, _id) =
        Input2.empty
        |> Input2.addBinding(
             [Keycode(1, Modifiers.none), Keycode(3, Modifiers.none)],
             _ => true,
             "payloadAC",
           );

      let (bindings, _id) =
        bindings
        |> Input2.addBinding(
             [Keycode(1, Modifiers.none)],
             _ => true,
             "payloadA",
           );

      let (bindings, effects) = Input2.keyDown(aKeyNoModifiers, bindings);

      expect.equal(effects, []);

      let (bindings, effects) = Input2.keyDown(aKeyNoModifiers, bindings);

      expect.equal(effects, [Execute("payloadA")]);

      let (_bindings, effects) = Input2.keyDown(cKeyNoModifiers, bindings);

      expect.equal(effects, [Execute("payloadAC")]);
    });
    test("partial match with unhandled", ({expect}) => {
      let (bindings, _id) =
        Input2.empty
        |> Input2.addBinding(
             [Keycode(1, Modifiers.none), Keycode(3, Modifiers.none)],
             _ => true,
             "payloadAC",
           );

      let (bindings, _id) =
        bindings
        |> Input2.addBinding(
             [Keycode(1, Modifiers.none)],
             _ => true,
             "payloadA",
           );

      let (bindings, effects) = Input2.keyDown(aKeyNoModifiers, bindings);

      expect.equal(effects, []);

      let (bindings, effects) = Input2.keyDown(bKeyNoModifiers, bindings);

      expect.equal(
        effects,
        [Unhandled(bKeyNoModifiers), Execute("payloadA")],
      );
    });
  });
  describe("key matching", ({test, _}) => {
    test("matches keycode", ({expect}) => {
      let (bindings, _id) =
        Input2.empty
        |> Input2.addBinding(
             [Keycode(1, Modifiers.none)],
             _ => true,
             "payload1",
           );

      let (_bindings, effects) = Input2.keyDown(aKeyNoModifiers, bindings);

      expect.equal(effects, [Execute("payload1")]);
    });
    test("key with no matches is unhandled", ({expect}) => {
      let bindings = Input2.empty;

      let (_bindings, effects) = Input2.keyDown(aKeyNoModifiers, bindings);

      expect.equal(effects, [Unhandled(aKeyNoModifiers)]);
    });
  });
});
