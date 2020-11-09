open Oni2_KeyboardLayout;
open TestFramework;

describe("Keymap", ({test, _}) => {
  test("getCurrentKeymap", ({expect, _}) => {
    let keymap = Keymap.getCurrent();

    expect.bool(Keymap.size(keymap) > 0).toBeTrue();
  });

  test("entryOfKey", ({expect, _}) => {
    let keymap = Keymap.getCurrent();
    let entry = Keymap.entryOfKey(keymap, "KeyS");

    expect.option(entry).toBeSome();

    let entry = Option.get(entry);

    expect.string(Keymap.(entry.unmodified)).toEqual("s");
    expect.string(Keymap.(entry.withShift)).toEqual("S");
  });
});
