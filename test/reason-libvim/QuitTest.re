open TestFramework;
open Vim;

let resetBuffer = () => Helpers.resetBuffer("test/testfile.txt");
let input = s => ignore(Vim.input(s));
let key = s => ignore(Vim.key(s));

describe("Quit", ({test, _}) => {
  test("q command", ({expect, _}) => {
    let b = resetBuffer();

    let updates = ref([]);

    let dispose =
      onQuit((quitType, forced) =>
        updates := [(quitType, forced), ...updates^]
      );

    let (_context: Context.t, _effects: list(Vim.Effect.t)) = command("q");
    let (qt, forced) = List.hd(updates^);

    expect.int(List.length(updates^)).toBe(1);

    switch (qt) {
    | QuitOne(buf) => expect.bool(buf == b).toBe(true)
    | _ => expect.string("Should've been QuitOne").toEqual("")
    };

    expect.bool(forced).toBe(false);

    dispose();
  });
  test("q input", ({expect, _}) => {
    let b = resetBuffer();

    let updates = ref([]);

    let dispose =
      onQuit((quitType, forced) =>
        updates := [(quitType, forced), ...updates^]
      );

    input(":");
    input("q");
    key("<cr>");
    let (qt, forced) = List.hd(updates^);

    expect.int(List.length(updates^)).toBe(1);

    switch (qt) {
    | QuitOne(buf) => expect.bool(buf == b).toBe(true)
    | _ => expect.string("Should've been QuitOne").toEqual("")
    };

    expect.bool(forced).toBe(false);

    dispose();
  });
  test("q!", ({expect, _}) => {
    let b = resetBuffer();

    let updates = ref([]);

    let dispose =
      onQuit((quitType, forced) =>
        updates := [(quitType, forced), ...updates^]
      );

    ignore(command("q!"): (Context.t, list(Effect.t)));
    let (qt, forced) = List.hd(updates^);

    expect.int(List.length(updates^)).toBe(1);

    switch (qt) {
    | QuitOne(buf) => expect.bool(buf == b).toBe(true)
    | _ => expect.string("Should've been QuitOne").toEqual("")
    };

    expect.bool(forced).toBe(true);

    dispose();
  });
  test("qall", ({expect, _}) => {
    let _ = resetBuffer();

    let updates = ref([]);

    let dispose =
      onQuit((quitType, forced) =>
        updates := [(quitType, forced), ...updates^]
      );

    ignore(command("qall"): (Context.t, list(Effect.t)));
    let (qt, forced) = List.hd(updates^);

    expect.int(List.length(updates^)).toBe(1);

    switch (qt) {
    | QuitAll => expect.bool(true).toBe(true)
    | _ => expect.string("Should've been QuitAll").toEqual("")
    };

    expect.bool(forced).toBe(false);

    dispose();
  });
});
